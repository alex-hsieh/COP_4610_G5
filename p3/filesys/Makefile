CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude 
TARGET = bin/filesys
SRC = src/main.c src/fat32.c src/shell.c src/commands.c src/lexer.c
OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
		mkdir -p bin
			$(CC) $(OBJ) -o $(TARGET)

src/%.o: src/%.c
		$(CC) $(CFLAGS) -c $< -o $@

clean:
		rm -f src/*.o $(TARGET)
			rm -rf bin

.PHONY: all clean
