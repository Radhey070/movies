#pragma once
#include "models.h"

int findMovieIndex(const std::vector<Movie>& movies, int id);
double expectedOccupancy(const Movie& movie, int start);
Show makeShow(const Movie& movie, const Screen& screen, int start, int turnaround);
double dailyDemand(const Movie& movie);
void applyRemainingDemand(Show& show, const Movie& movie, const Screen& screen,
                          double remaining, int turnaround);
void mergeSort(std::vector<Show>& shows, int left, int right);
