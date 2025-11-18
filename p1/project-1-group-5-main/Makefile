# dirs & names
SRC := src
OBJ := obj
BIN := bin
EXECUTABLE := shell
EXEC := $(BIN)/$(EXECUTABLE)

# toolchain
CC := gcc
INCS := -Iinclude/
CFLAGS := -g -Wall -Wextra -Wpedantic -std=c99 $(INCS) -MMD -MP
LDFLAGS :=

# sources/objects
SRCS := $(wildcard $(SRC)/*.c)
OBJS := $(patsubst $(SRC)/%.c,$(OBJ)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

all: $(EXEC)

# link (ensure bin exists)
$(EXEC): $(OBJS) | $(BIN)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)

# compile (ensure obj exists)
$(OBJ)/%.o: $(SRC)/%.c | $(OBJ)
	$(CC) $(CFLAGS) -c $< -o $@

# create dirs if missing
$(BIN) $(OBJ):
	mkdir -p $@

run: $(EXEC)
	$(EXEC)

clean:
	rm -f $(OBJ)/*.o $(DEPS) $(EXEC)

-include $(DEPS)

.PHONY: all run clean
