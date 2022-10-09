SRC_DIR := src
OBJ_DIR := build/obj

EXE := chess.out
SRC := $(wildcard $(SRC_DIR)/*.c)
OBJ := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

CC := gcc
CPPFLAGS := -Iinclude -MMD -MP
CFLAGS := -ansi -Wall -g -Wwrite-strings -Wshadow -pedantic-errors -fstack-protector-all -Wextra
CCFLAGS := -std=c99 -DLOG -O3 -march=native
LDFLAGS := -pthread
LDLIBS := -lm

.PHONY: all clean

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) $(CCFLAGS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CCFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $@

clean:
	@$(RM) -r $(OBJ_DIR)
