CC=gcc
CFLAGS=-Wall -Wextra -Werror -std=c17 -g
INCLUDES=-Iinclude
SRC=src/main.c src/parse.c src/exec.c src/builtins.c src/jobs.c src/path.c src/expand.c
OBJ=$(SRC:.c=.o)
bin/shell: $(OBJ)
	mkdir -p bin
	$(CC) $(CFLAGS) $(OBJ) -o bin/shell
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
.PHONY: clean
clean: ; rm -f $(OBJ) bin/shell
