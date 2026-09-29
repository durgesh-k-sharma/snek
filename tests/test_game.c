#include "game.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

static void check(int condition, const char *message) {
  if (!condition) {
    fprintf(stderr, "FAIL: %s\n", message);
    failures++;
  }
}

static game_t *load_text(const char *text) {
  FILE *fp = tmpfile();
  if (fp == NULL) return NULL;
  if (fputs(text, fp) == EOF || fflush(fp) != 0 || fseek(fp, 0, SEEK_SET) != 0) {
    fclose(fp);
    return NULL;
  }
  game_t *game = load_board(fp);
  fclose(fp);
  return game;
}

static int no_food(game_t *game) {
  (void)game;
  return 0;
}

static void test_default_game(void) {
  game_t *game = create_default_game();
  check(game != NULL, "default game allocation");
  if (game == NULL) return;
  check(game->num_rows == 18, "default board height");
  check(strlen(game->board[0]) == 21, "default board row width and newline");
  check(game->num_snakes == 1, "default snake count");
  check(game->snakes[0].tail_row == 2 && game->snakes[0].tail_col == 2, "default tail position");
  check(game->snakes[0].head_row == 2 && game->snakes[0].head_col == 4, "default head position");
  update_game(game, no_food);
  check(game->board[2][5] == 'D', "snake advances one square");
  check(game->board[2][2] == ' ', "tail advances one square");
  check(game->board[2][3] == 'd', "tail follows the body direction");
  free_game(game);
  free_game(NULL);
}

static void test_board_stream(void) {
  game_t *game = load_text("########\n# d>D  #\n#    * #\n########\n");
  check(game != NULL, "load board from stream");
  if (game == NULL) return;
  check(game->num_rows == 4, "loaded row count");
  check(strlen(game->board[1]) == 9, "loaded row preserves newline");
  game = initialize_snakes(game);
  check(game != NULL, "initialize loaded snakes");
  if (game != NULL) {
    check(game->num_snakes == 1, "loaded snake count");
    check(game->snakes[0].head_row == 1 && game->snakes[0].head_col == 4, "loaded head position");
    update_game(game, no_food);
    check(game->board[1][5] == 'D', "loaded snake moves");
    free_game(game);
  }
  FILE *empty = tmpfile();
  check(empty != NULL, "create empty test stream");
  if (empty != NULL) {
    check(load_board(empty) == NULL, "empty stream is rejected");
    fclose(empty);
  }
}

static void test_food_growth(void) {
  game_t *game = load_text("########\n# d>D* #\n########\n");
  check(game != NULL, "load food board");
  if (game == NULL) return;
  initialize_snakes(game);
  update_game(game, no_food);
  check(game->board[1][5] == 'D', "snake enters fruit square");
  check(game->board[1][2] == 'd', "eating preserves the tail");
  free_game(game);
}

int main(void) {
  test_default_game();
  test_board_stream();
  test_food_growth();
  if (failures != 0) return EXIT_FAILURE;
  puts("All unit tests passed.");
  return EXIT_SUCCESS;
}
