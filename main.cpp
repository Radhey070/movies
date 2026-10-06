#include "scheduler.h"
#include "algorithms.h"
#include "demo_data.h"
#include <iostream>
#include <iomanip>

// DISPLAY ONLY: show both schedules so the difference is easy to explain.
void printSchedule(const std::string& title, const ScheduleResult& schedule,
                   const Metrics& metrics, const std::vector<Movie>& movies) {
    std::cout << "\n" << title << "\n";
    std::cout << "Screen  Start  End    Movie                  Revenue (Rs)\n";
    for (int i = 0; i < (int)schedule.shows.size(); i++) {
        const Show& show = schedule.shows[i];
        int m = findMovieIndex(movies, show.movieId);
        std::cout << std::left << std::setw(8) << show.screenId
                  << formatTime(show.startMinutes) << "  " << formatTime(show.endMinutes)
                  << "  " << std::setw(23) << movies[m].name << show.expectedRevenue << "\n";
    }
    std::cout << "Total revenue: Rs " << metrics.totalRevenue << "\n";
    std::cout << "Shows: " << metrics.totalShows << "\n";
    std::cout << "Expected viewers: " << metrics.totalAudience << "\n";
    std::cout << "Seat utilization: " << metrics.seatUtilizationPercent << "%\n";
    std::cout << "Screen utilization: " << metrics.screenUtilizationPercent << "%\n";
    std::cout << "Prime-time utilization: " << metrics.primeTimeUtilizationPercent << "%\n";
}

int main() {
    Theatre theatre;
    std::vector<Screen> screens;
    std::vector<Movie> movies;
    int grid;
    loadInputs(theatre, screens, movies, grid);

    EngineResult result = runEngine(theatre, screens, movies, grid);
    if (!result.validationErrors.empty()) {
        for (int i = 0; i < (int)result.validationErrors.size(); i++)
            std::cout << result.validationErrors[i] << "\n";
        return 1;
    }
    std::cout << std::fixed << std::setprecision(2);
    std::cout << theatre.name << " | " << formatTime(theatre.openingMinutes)
              << " to " << formatTime(theatre.closingMinutes) << "\n";
    std::cout << "Turnaround: " << theatre.turnaroundMinutes << " min; grid: " << grid << " min\n";
    printSchedule("BASELINE", result.baseline, result.baselineMetrics, movies);
    printSchedule("GREEDY", result.optimized, result.optimizedMetrics, movies);
    std::cout << "\nProjected revenue improvement: " << result.revenueImprovementPercent << "%\n";
    std::cout << "\nLast three accepted decisions (stack, newest first):\n";
    for (int i = 0; i < 3 && i < (int)result.optimized.recentDecisions.size(); i++) {
        const Show& show = result.optimized.recentDecisions[i];
        std::cout << movies[findMovieIndex(movies,show.movieId)].name << " on screen "
                  << show.screenId << " at " << formatTime(show.startMinutes) << "\n";
    }
    std::cout << "Greedy checks skipped: " << result.optimized.rejectedDueToOverlap
              << " overlaps, " << result.optimized.rejectedDueToDemand << " exhausted demand\n";
    return 0;
}
