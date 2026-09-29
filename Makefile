CC ?= cc
CFLAGS ?= -std=c99 -Wall -Wextra -Wconversion -g
LDFLAGS ?=

GAME_OBJECTS = src/game.o src/snake_utils.o

.PHONY: all test clean

all: snake interactive-snake

snake: src/snake.o $(GAME_OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

interactive-snake: src/interactive_snake.o $(GAME_OBJECTS)
	$(CC) $(LDFLAGS) -pthread -o $@ $^

unit-tests: tests/test_game.o $(GAME_OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

test: unit-tests snake
	./unit-tests
	./tools/run_regression_tests.sh
	./tools/test_cli.sh

src/%.o: src/%.c src/game.h src/snake_utils.h
	$(CC) $(CFLAGS) -c -o $@ $<

tests/%.o: tests/%.c src/game.h
	$(CC) $(CFLAGS) -Isrc -c -o $@ $<

clean:
	rm -f snake interactive-snake unit-tests src/*.o tests/*.o /tmp/snek-*-out.snk
