OBJS = $(wildcard src/*.c)

CC = gcc

CFLAGS = -Wall -Wpedantic -Wextra

LDFLAGS = -lSDL2 -lm

BIN = tree

all : $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(BIN) $(LDFLAGS)