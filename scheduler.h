#pragma once
#include "models.h"

// ---------- Result types ---------------------------------------------

struct ScheduleResult {
    std::vector<Show> shows;                 // accepted shows, sorted by screen then start time
    int rejectedDueToOverlap = 0;
    int rejectedDueToDemand = 0;
    std::vector<Show> recentDecisions; // Accepted decisions in reverse acceptance order.
};

struct MovieStat {
    int movieId;
    std::string movieName;
    int shows = 0;
    double expectedAudience = 0.0;
    double expectedRevenue = 0.0;
    double averageOccupancy = 0.0;
    int screensUsed = 0;
};

struct Metrics {
    double totalRevenue = 0.0;
    double totalAudience = 0.0;
    int totalShows = 0;
    double seatUtilizationPercent = 0.0;

    int totalScreenOperatingMinutes = 0;
    int scheduledScreenMinutes = 0;     // sum of movie runtimes only (turnaround excluded)
    int idleScreenMinutes = 0;
    double screenUtilizationPercent = 0.0;

    double primeTimeUtilizationPercent = 0.0; // 17:00-21:00 window

    std::vector<MovieStat> perMovie;
};

struct EngineResult {
    std::vector<std::string> validationErrors;
    ScheduleResult baseline;
    ScheduleResult optimized;
    Metrics baselineMetrics;
    Metrics optimizedMetrics;
    double revenueImprovementPercent = 0.0;
};

EngineResult runEngine(const Theatre& theatre, const std::vector<Screen>& screens,
                       const std::vector<Movie>& movies, int grid);
