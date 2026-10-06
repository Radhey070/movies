#include "algorithms.h"

// 1. LINEAR SEARCH: examine each movie until its ID matches.
int findMovieIndex(const std::vector<Movie>& movies, int id) {
    for (int i = 0; i < (int)movies.size(); i++) {
        if (movies[i].id == id) return i;
    }
    return -1;
}

// 2. DEMAND MODEL: these multipliers are assumptions, not measured sales.
double expectedOccupancy(const Movie& movie, int start) {
    int hour = (start / 60) % 24;
    double factor;
    if (hour < 12) factor = 0.60;
    else if (hour < 17) factor = 0.85;
    else if (hour < 21) factor = 1.15;
    else factor = 0.75;
    double occupancy = (movie.demandScore / 10.0) * factor;
    if (occupancy > 1.0) occupancy = 1.0;
    return occupancy;
}

// 3. BUILD ONE SHOW: the same formulas are used by both schedulers.
Show makeShow(const Movie& movie, const Screen& screen, int start, int turnaround) {
    Show show;
    show.movieId = movie.id;
    show.screenId = screen.id;
    show.startMinutes = start;
    show.endMinutes = start + movie.runtimeMinutes;
    show.expectedOccupancy = expectedOccupancy(movie, start);
    show.expectedAudience = screen.capacity * show.expectedOccupancy;
    show.expectedRevenue = show.expectedAudience * movie.basePrice * screen.priceMultiplier;
    show.revenuePerMinute = show.expectedRevenue / (movie.runtimeMinutes + turnaround);
    return show;
}

// A simple daily audience estimate: one demand point represents 100 viewers.
double dailyDemand(const Movie& movie) { return movie.demandScore * 100.0; }

// Shows of the same movie share that day's viewers; later shows cannot
// sell the same expected tickets again.
void applyRemainingDemand(Show& show, const Movie& movie, const Screen& screen,
                          double remaining, int turnaround) {
    if (show.expectedAudience > remaining) show.expectedAudience = remaining;
    show.expectedOccupancy = show.expectedAudience / screen.capacity;
    show.expectedRevenue = show.expectedAudience * movie.basePrice * screen.priceMultiplier;
    show.revenuePerMinute = show.expectedRevenue / (movie.runtimeMinutes + turnaround);
}

// 4. RANKING RULE: ties have fixed rules so repeated runs agree.
bool comesFirst(const Show& a, const Show& b) {
    if (a.revenuePerMinute != b.revenuePerMinute) return a.revenuePerMinute > b.revenuePerMinute;
    if (a.expectedRevenue != b.expectedRevenue) return a.expectedRevenue > b.expectedRevenue;
    if (a.expectedOccupancy != b.expectedOccupancy) return a.expectedOccupancy > b.expectedOccupancy;
    if (a.startMinutes != b.startMinutes) return a.startMinutes < b.startMinutes;
    if (a.movieId != b.movieId) return a.movieId < b.movieId;
    return a.screenId < b.screenId;
}

// 5. MERGE SORT: sort each half, then combine the sorted halves.
void mergeSort(std::vector<Show>& shows, int left, int right) {
    if (left >= right) return;
    int middle = left + (right - left) / 2;
    mergeSort(shows, left, middle);
    mergeSort(shows, middle + 1, right);

    std::vector<Show> merged;
    int i = left;
    int j = middle + 1;
    while (i <= middle && j <= right) {
        if (comesFirst(shows[j], shows[i])) merged.push_back(shows[j++]);
        else merged.push_back(shows[i++]);
    }
    while (i <= middle) merged.push_back(shows[i++]);
    while (j <= right) merged.push_back(shows[j++]);
    for (int k = 0; k < (int)merged.size(); k++) shows[left + k] = merged[k];
}
