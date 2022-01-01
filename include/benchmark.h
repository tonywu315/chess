#ifndef BENCHMARK_H
#define BENCHMARK_H

#include "constants.h"

void benchmark(Board *board, int depth);
void perft(Board *board, int depth, U64 *nodes);

#endif
