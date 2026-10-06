#include "scheduler.h"
#include "algorithms.h"
#include "data_structures.h"
#include "metrics.h"
#include <cmath>

// 1. VALIDATE: stop before scheduling if the input is impossible.
std::vector<std::string> validateInput(const Theatre& t, const std::vector<Screen>& screens,
                                      const std::vector<Movie>& movies, int grid) {
    std::vector<std::string> errors;
    if (t.openingMinutes < 0 || t.closingMinutes > 1440 || t.closingMinutes <= t.openingMinutes)
        errors.push_back("Use opening and closing times within one day; closing must follow opening.");
    if (t.turnaroundMinutes < 0 || t.turnaroundMinutes > 1440)
        errors.push_back("Turnaround must be between 0 and 1440 minutes.");
    if (grid < 5 || grid > 1440) errors.push_back("Candidate grid must be between 5 and 1440 minutes.");
    if (screens.empty() || screens.size() > 20) errors.push_back("Enter 1 to 20 screens.");
    if (movies.empty() || movies.size() > 50) errors.push_back("Enter 1 to 50 movies.");
    int available = 0;
    for (int i = 0; i < (int)screens.size(); i++) {
        if (screens[i].available) available++;
        if (screens[i].capacity < 1 || screens[i].capacity > 10000 ||
            !std::isfinite(screens[i].priceMultiplier) || screens[i].priceMultiplier < 1 || screens[i].priceMultiplier > 100)
            errors.push_back("Screen capacity must be 1-10000 and multiplier 1-100.");
        for (int j = 0; j < i; j++) {
            if (screens[i].id == screens[j].id) errors.push_back("Duplicate screen ID.");
        }
    }
    if (available == 0) errors.push_back("Enable at least one screen.");
    for (int i = 0; i < (int)movies.size(); i++) {
        const Movie& m = movies[i];
        if (m.runtimeMinutes < 1 || m.runtimeMinutes > t.closingMinutes - t.openingMinutes)
            errors.push_back("Each movie must fit within the operating day.");
        if (!std::isfinite(m.basePrice) || m.basePrice <= 0 || m.basePrice > 1000000 ||
            m.demandScore < 1 || m.demandScore > 10)
            errors.push_back("Check price (0-1000000) and demand (1-10).");
        for (int j = 0; j < i; j++) {
            if (m.id == movies[j].id) errors.push_back("Duplicate movie ID.");
        }
    }
    return errors;
}

// 2. CANDIDATES: try every screen, movie and grid start time.
std::vector<Show> generateCandidates(const Theatre& t, const std::vector<Screen>& screens,
                                     const std::vector<Movie>& movies, int grid) {
    std::vector<Show> candidates;
    MovieList roster;
    for (int m = 0; m < (int)movies.size(); m++) roster.append(movies[m]);
    for (int s = 0; s < (int)screens.size(); s++) {
        if (!screens[s].available) continue;
        for (MovieNode* node = roster.head; node != nullptr; node = node->next) {
            Movie movie = node->movie;
            for (int start = t.openingMinutes; start + movie.runtimeMinutes <= t.closingMinutes; start += grid)
                candidates.push_back(makeShow(movie, screens[s], start, t.turnaroundMinutes));
        }
    }
    return candidates;
}

// 3. BASELINE: use the next movie in the circular rotation if it fits.
ScheduleResult runBaseline(const Theatre& t, const std::vector<Screen>& screens,
                            const std::vector<Movie>& movies) {
    ScheduleResult result;
    ShowList timeline;
    std::vector<double> remaining;
    for (const Movie& movie : movies) remaining.push_back(dailyDemand(movie));
    for (int s = 0; s < (int)screens.size(); s++) {
        if (!screens[s].available) continue;
        CircularQueue rotation((int)movies.size());
        int start = t.openingMinutes;
        int failures = 0;
        while (failures < (int)movies.size()) {
            int m = rotation.nextMovie();
            bool tooLate = start + movies[m].runtimeMinutes > t.closingMinutes;
            if (remaining[m] <= 0.000001 || tooLate) { failures++; continue; }
            Show show = makeShow(movies[m], screens[s], start, t.turnaroundMinutes);
            applyRemainingDemand(show, movies[m], screens[s], remaining[m], t.turnaroundMinutes);
            show.reason = "Next movie in the fixed rotation with remaining daily demand.";
            timeline.insertSorted(show);
            remaining[m] -= show.expectedAudience;
            start = show.endMinutes + t.turnaroundMinutes;
            failures = 0;
        }
    }
    result.shows = timeline.toVector();
    return result;
}

// 4. OVERLAP: a screen cannot play two movies at once, including the buffer.
bool hasConflict(const Show& candidate, const ShowList& accepted, int gap) {
    for (ShowNode* node = accepted.head; node != nullptr; node = node->next) {
        const Show& other = node->show;
        if (candidate.screenId == other.screenId &&
            candidate.startMinutes < other.endMinutes + gap &&
            other.startMinutes < candidate.endMinutes + gap) return true;
    }
    return false;
}

// 5. GREEDY: rank, enqueue, check, accept and record the decision.
ScheduleResult runGreedy(const Theatre& t, const std::vector<Screen>& screens, const std::vector<Movie>& movies,
                         std::vector<Show> candidates) {
    ScheduleResult result;
    std::vector<double> remaining;
    for (const Movie& movie : movies) remaining.push_back(dailyDemand(movie));
    MovieBST movieIndex;
    for (int i = 0; i < (int)movies.size(); i++) movieIndex.insert(movies[i].id, i);
    ShowList timeline;
    DecisionStack history;
    while (true) {
        // Recalculate value after each accepted show because its movie now has
        // fewer viewers left. Sort, then use the queue until a show fits.
        std::vector<Show> ranked;
        for (Show show : candidates) {
            int m = movieIndex.find(show.movieId);
            if (remaining[m] <= 0.000001) { result.rejectedDueToDemand++; continue; }
            for (const Screen& screen : screens) {
                if (screen.id == show.screenId) {
                    applyRemainingDemand(show, movies[m], screen, remaining[m], t.turnaroundMinutes);
                    break;
                }
            }
            ranked.push_back(show);
        }
        if (ranked.empty()) break;
        mergeSort(ranked, 0, (int)ranked.size() - 1);
        ShowQueue processing;
        for (const Show& show : ranked) processing.enqueue(show);
        Show show;
        bool found = false;
        while (!processing.empty()) {
            show = processing.dequeue();
            if (hasConflict(show, timeline, t.turnaroundMinutes)) {
                result.rejectedDueToOverlap++;
                continue;
            }
            found = true;
            break;
        }
        if (!found) break;
        int m = movieIndex.find(show.movieId);
        show.reason = "Highest current revenue/minute; screen free and daily viewers remain.";
        timeline.insertSorted(show);
        remaining[m] -= show.expectedAudience;
        history.push(show);
    }
    result.shows = timeline.toVector();
    // Popping this history does not remove scheduled shows: it builds a report.
    while (!history.empty()) result.recentDecisions.push_back(history.pop());
    return result;
}

// 7. COORDINATOR: read this function first during the viva.
EngineResult runEngine(const Theatre& t, const std::vector<Screen>& inputScreens,
                       const std::vector<Movie>& movies, int grid) {
    EngineResult result;
    result.validationErrors = validateInput(t, inputScreens, movies, grid);
    if (!result.validationErrors.empty()) return result;
    std::vector<Screen> screens = inputScreens;
    // Insertion sort screens by ID so the baseline has a fixed order.
    for (int i = 1; i < (int)screens.size(); i++) {
        Screen current = screens[i];
        int j = i - 1;
        while (j >= 0 && screens[j].id > current.id) { screens[j + 1] = screens[j]; j--; }
        screens[j + 1] = current;
    }
    std::vector<Show> candidates = generateCandidates(t, screens, movies, grid);
    result.baseline = runBaseline(t, screens, movies);
    result.optimized = runGreedy(t, screens, movies, candidates);
    result.baselineMetrics = computeMetrics(t, screens, movies, result.baseline);
    result.optimizedMetrics = computeMetrics(t, screens, movies, result.optimized);
    double base = result.baselineMetrics.totalRevenue;
    if (base > 0) result.revenueImprovementPercent = 100.0 * (result.optimizedMetrics.totalRevenue - base) / base;
    return result;
}
