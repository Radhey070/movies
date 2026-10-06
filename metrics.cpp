#include "metrics.h"
#include <algorithm>

// 6. METRICS: add the results; these totals do not influence either algorithm.
Metrics computeMetrics(const Theatre& t, const std::vector<Screen>& screens,
                        const std::vector<Movie>& movies, const ScheduleResult& schedule) {
    Metrics metrics;
    double offeredSeats = 0;
    int primeMinutes = 0;
    int availableScreens = 0;
    metrics.totalShows = (int)schedule.shows.size();
    for (int s = 0; s < (int)screens.size(); s++) {
        if (screens[s].available) availableScreens++;
    }
    metrics.totalScreenOperatingMinutes = availableScreens * (t.closingMinutes - t.openingMinutes);
    for (int m = 0; m < (int)movies.size(); m++) {
        MovieStat stat;
        stat.movieId = movies[m].id;
        stat.movieName = movies[m].name;
        std::vector<int> used(screens.size(), 0);
        for (int i = 0; i < (int)schedule.shows.size(); i++) {
            const Show& show = schedule.shows[i];
            if (show.movieId != movies[m].id) continue;
            stat.shows++;
            stat.expectedRevenue += show.expectedRevenue;
            stat.expectedAudience += show.expectedAudience;
            stat.averageOccupancy += show.expectedOccupancy;
            metrics.totalRevenue += show.expectedRevenue;
            metrics.totalAudience += show.expectedAudience;
            metrics.scheduledScreenMinutes += show.endMinutes - show.startMinutes;
            primeMinutes += std::max(0, std::min(show.endMinutes, 1260) - std::max(show.startMinutes, 1020));
            for (int s = 0; s < (int)screens.size(); s++) {
                if (show.screenId == screens[s].id) {
                    offeredSeats += screens[s].capacity;
                    if (!used[s]) { used[s] = true; stat.screensUsed++; }
                }
            }
        }
        if (stat.shows > 0) {
            stat.averageOccupancy /= stat.shows;
            metrics.perMovie.push_back(stat);
        }
    }
    metrics.idleScreenMinutes = metrics.totalScreenOperatingMinutes - metrics.scheduledScreenMinutes;
    metrics.screenUtilizationPercent = 100.0 * metrics.scheduledScreenMinutes / metrics.totalScreenOperatingMinutes;
    if (offeredSeats > 0) metrics.seatUtilizationPercent = 100.0 * metrics.totalAudience / offeredSeats;
    int primeWindow = std::max(0, std::min(t.closingMinutes, 1260) - std::max(t.openingMinutes, 1020));
    if (primeWindow > 0) metrics.primeTimeUtilizationPercent = 100.0 * primeMinutes / (primeWindow * availableScreens);
    return metrics;
}

