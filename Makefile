EXE := chess
CC := gcc
CFLAGS := -std=c99 -Wall -g -Wwrite-strings -Wshadow -pedantic-errors -fstack-protector-all -Wextra
LDFLAGS := -pthread

.PHONY: all

all:
	$(CC) $(CFLAGS) -O3 -march=native $(LDFLAGS) -Iinclude src/*.c -o $(EXE)
