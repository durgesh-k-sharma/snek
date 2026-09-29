#include <stdio.h>
#include <string.h>

#include "snake_utils.h"
#include "game.h"

int main(int argc, char *argv[]) {
  bool io_stdin = false;
  char *in_filename = NULL;
  char *out_filename = NULL;
  game_t *game = NULL;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-i") == 0 && i < argc - 1) {
      if (io_stdin) {
        fprintf(stderr, "Usage: %s [-i filename | --stdin] [-o filename]\n", argv[0]);
        return 1;
      }
      in_filename = argv[i + 1];
      i++;
      continue;
    } else if (strcmp(argv[i], "--stdin") == 0) {
      if (in_filename != NULL) {
        fprintf(stderr, "Usage: %s [-i filename | --stdin] [-o filename]\n", argv[0]);
        return 1;
      }
      io_stdin = true;
      continue;
    }
    if (strcmp(argv[i], "-o") == 0 && i < argc - 1) {
      out_filename = argv[i + 1];
      i++;
      continue;
    }
    fprintf(stderr, "Usage: %s [-i filename | --stdin] [-o filename]\n", argv[0]);
    return 1;
  }

  if (in_filename != NULL) {
    FILE *fp = fopen(in_filename, "r");
    if (fp == NULL) {
      perror(in_filename);
      return -1;
    }
    game = load_board(fp);
    fclose(fp);
    if (game != NULL) game = initialize_snakes(game);
  } else if (io_stdin) {
    game = load_board(stdin);
    if (game != NULL) game = initialize_snakes(game);
  } else {
    game = create_default_game();
  }
  if (game == NULL) {
    fprintf(stderr, "Could not load or create the game board.\n");
    return 1;
  }
  update_game(game, deterministic_food);
  if (out_filename != NULL) {
    FILE *fp = fopen(out_filename, "w");
    if (fp == NULL) {
      perror(out_filename);
      free_game(game);
      return 1;
    }
    print_board(game, fp);
    int failed = ferror(fp);
    if (fclose(fp) != 0) failed = 1;
    if (failed) {
      fprintf(stderr, "Could not write board to %s.\n", out_filename);
      free_game(game);
      return 1;
    }
  } else {
    print_board(game, stdout);
  }
  free_game(game);
  return 0;
}
