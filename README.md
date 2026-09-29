# Snake

Snake is a terminal game written in C. It includes a step-based simulator and an interactive mode with keyboard controls.

## Build

You need a C compiler, GNU Make, and POSIX threads.

```sh
make
```

This builds `snake` and `interactive-snake`. Run `make test` to run the unit tests, compare every included board fixture, and check the command-line input modes. The regression checks live in `tools/run_regression_tests.sh` and `tools/test_cli.sh`. Run `make clean` to remove build outputs.

## Run the simulator

Run one step from the default board and print the result:

```sh
./snake
```

Load a board from a file, from standard input, or write the result to a file:

```sh
./snake -i board.snk
./snake --stdin < board.snk
./snake -i board.snk -o next-step.snk
```

Each run advances the board by one turn. A missing input file returns a nonzero exit status. Boards are text grids. Rows may have different widths, and each row ends with a newline. Use `#` for walls, `*` for fruit, and directional letters or arrows for snake tails, bodies, and heads.

## Play interactively

Start a new game with:

```sh
./interactive-snake
```

Use `w`, `a`, `s`, and `d` to steer the snake. Press `[` to slow the game and `]` to speed it up. To start from a saved board, pass `-i board.snk`. Pass `-d seconds` to set the initial delay between turns.

## Run tests

```sh
make test
```

The tests cover game setup, board loading, snake initialization, movement, growth, and the saved regression boards in `tests/`.
