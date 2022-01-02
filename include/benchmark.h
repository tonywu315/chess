#ifndef BENCHMARK_H
#define BENCHMARK_H

#include "types.h"

typedef unsigned long long U64;

void benchmark(Board *board, int depth);
void perft(Board *board, int depth, U64 *nodes);

#endif
