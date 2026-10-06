#pragma once
#include "scheduler.h"

Metrics computeMetrics(const Theatre& theatre, const std::vector<Screen>& screens,
                       const std::vector<Movie>& movies, const ScheduleResult& schedule);
