#include "game.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "snake_utils.h"

static bool is_tail(char c);
static bool is_head(char c);
static bool is_snake(char c);
static char body_to_tail(char c);
static char head_to_body(char c);
static unsigned int get_next_row(unsigned int row, char c);
static unsigned int get_next_col(unsigned int col, char c);
static void find_head(game_t *game, unsigned int snum);
static char next_square(game_t *game, unsigned int snum);
static void update_tail(game_t *game, unsigned int snum);
static void update_head(game_t *game, unsigned int snum);

game_t *create_default_game(void) {
  game_t *game = calloc(1, sizeof(*game));
  if (game == NULL) return NULL;
  game->num_rows = 18;
  game->board = calloc(game->num_rows, sizeof(*game->board));
  game->snakes = calloc(1, sizeof(*game->snakes));
  if (game->board == NULL || game->snakes == NULL) {
    free_game(game);
    return NULL;
  }
  for (unsigned int i = 0; i < game->num_rows; i++) {
    game->board[i] = malloc(22);
    if (game->board[i] == NULL) {
      free_game(game);
      return NULL;
    }
    memset(game->board[i], ' ', 20);
    game->board[i][19] = '#';
    game->board[i][20] = '\n';
    game->board[i][21] = '\0';
    if (i == 0 || i == game->num_rows - 1) memset(game->board[i], '#', 20);
    else game->board[i][0] = '#';
  }
  memcpy(game->board[2] + 2, "d>D    *", 8);
  game->num_snakes = 1;
  game->snakes[0] = (snake_t){.tail_row = 2, .tail_col = 2, .head_row = 2, .head_col = 4, .live = true};
  return game;
}

void free_game(game_t *game) {
  if (game == NULL) return;
  if (game->board != NULL) {
    for (unsigned int i = 0; i < game->num_rows; i++) free(game->board[i]);
    free(game->board);
  }
  free(game->snakes);
  free(game);
}

void print_board(game_t *game, FILE *fp) {
  if (game == NULL || fp == NULL) return;
  for (unsigned int row = 0; row < game->num_rows; row++) fputs(game->board[row], fp);
}

void save_board(game_t *game, char *filename) {
  if (filename == NULL) return;
  FILE *fp = fopen(filename, "w");
  if (fp == NULL) return;
  print_board(game, fp);
  fclose(fp);
}

char get_board_at(game_t *game, unsigned int row, unsigned int col) { return game->board[row][col]; }

static bool is_tail(char c) { return c != '\0' && strchr("wasd", c) != NULL; }
static bool is_head(char c) { return c != '\0' && strchr("WASDx", c) != NULL; }
static bool is_snake(char c) { return c != '\0' && strchr("wasd^<v>WASDx", c) != NULL; }

static char body_to_tail(char c) {
  switch (c) {
    case '^': return 'w';
    case '<': return 'a';
    case 'v': return 's';
    case '>': return 'd';
    default: return '?';
  }
}

static char head_to_body(char c) {
  switch (c) {
    case 'W': return '^';
    case 'A': return '<';
    case 'S': return 'v';
    case 'D': return '>';
    default: return '?';
  }
}

static unsigned int get_next_row(unsigned int row, char c) {
  if (c == 'v' || c == 's' || c == 'S') return row + 1;
  if ((c == '^' || c == 'w' || c == 'W') && row > 0) return row - 1;
  return row;
}

static unsigned int get_next_col(unsigned int col, char c) {
  if (c == '>' || c == 'd' || c == 'D') return col + 1;
  if ((c == '<' || c == 'a' || c == 'A') && col > 0) return col - 1;
  return col;
}

static char next_square(game_t *game, unsigned int snum) {
  snake_t *snake = &game->snakes[snum];
  char direction = game->board[snake->head_row][snake->head_col];
  return game->board[get_next_row(snake->head_row, direction)][get_next_col(snake->head_col, direction)];
}

static void update_head(game_t *game, unsigned int snum) {
  snake_t *snake = &game->snakes[snum];
  unsigned int row = snake->head_row, col = snake->head_col;
  char direction = game->board[row][col];
  game->board[row][col] = head_to_body(direction);
  snake->head_row = get_next_row(row, direction);
  snake->head_col = get_next_col(col, direction);
  game->board[snake->head_row][snake->head_col] = direction;
}

static void update_tail(game_t *game, unsigned int snum) {
  snake_t *snake = &game->snakes[snum];
  unsigned int row = snake->tail_row, col = snake->tail_col;
  char direction = game->board[row][col];
  unsigned int next_row = get_next_row(row, direction), next_col = get_next_col(col, direction);
  char next = game->board[next_row][next_col];
  game->board[row][col] = ' ';
  game->board[next_row][next_col] = body_to_tail(next);
  snake->tail_row = next_row;
  snake->tail_col = next_col;
}

void update_game(game_t *game, int (*add_food)(game_t *game)) {
  if (game == NULL) return;
  for (unsigned int i = 0; i < game->num_snakes; i++) {
    snake_t *snake = &game->snakes[i];
    if (!snake->live) continue;
    char destination = next_square(game, i);
    if (destination == '#' || is_snake(destination)) {
      game->board[snake->head_row][snake->head_col] = 'x';
      snake->live = false;
      continue;
    }
    update_head(game, i);
    if (destination == '*') {
      if (add_food != NULL) add_food(game);
    } else {
      update_tail(game, i);
    }
  }
}

static char *read_line(FILE *fp) {
  size_t used = 0, capacity = 128;
  char *line = malloc(capacity);
  if (line == NULL) return NULL;
  char buffer[128];
  while (fgets(buffer, sizeof(buffer), fp) != NULL) {
    size_t count = strlen(buffer);
    if (used + count + 1 > capacity) {
      size_t next_capacity = capacity;
      while (used + count + 1 > next_capacity) next_capacity *= 2;
      char *next = realloc(line, next_capacity);
      if (next == NULL) { free(line); return NULL; }
      line = next;
      capacity = next_capacity;
    }
    memcpy(line + used, buffer, count);
    used += count;
    if (count > 0 && buffer[count - 1] == '\n') break;
  }
  if (used == 0) { free(line); return NULL; }
  line[used] = '\0';
  return line;
}

game_t *load_board(FILE *fp) {
  if (fp == NULL) return NULL;
  game_t *game = calloc(1, sizeof(*game));
  if (game == NULL) return NULL;
  size_t capacity = 8;
  game->board = calloc(capacity, sizeof(*game->board));
  if (game->board == NULL) { free(game); return NULL; }
  char *line;
  while ((line = read_line(fp)) != NULL) {
    if (game->num_rows == capacity) {
      size_t next_capacity = capacity * 2;
      char **next = realloc(game->board, next_capacity * sizeof(*next));
      if (next == NULL) { free(line); free_game(game); return NULL; }
      game->board = next;
      capacity = next_capacity;
    }
    game->board[game->num_rows++] = line;
  }
  if (ferror(fp) || game->num_rows == 0) { free_game(game); return NULL; }
  return game;
}

static void find_head(game_t *game, unsigned int snum) {
  snake_t *snake = &game->snakes[snum];
  unsigned int row = snake->tail_row, col = snake->tail_col;
  char direction = game->board[row][col];
  row = get_next_row(row, direction);
  col = get_next_col(col, direction);
  while (row < game->num_rows && col < strlen(game->board[row])) {
    char cell = game->board[row][col];
    if (is_head(cell)) {
      snake->head_row = row;
      snake->head_col = col;
      return;
    }
    row = get_next_row(row, cell);
    col = get_next_col(col, cell);
  }
}

game_t *initialize_snakes(game_t *game) {
  if (game == NULL) return NULL;
  unsigned int count = 0;
  for (unsigned int row = 0; row < game->num_rows; row++)
    for (unsigned int col = 0; game->board[row][col] != '\0' && game->board[row][col] != '\n'; col++)
      if (is_tail(game->board[row][col])) count++;
  snake_t *snakes = count == 0 ? NULL : calloc(count, sizeof(*snakes));
  if (count != 0 && snakes == NULL) return NULL;
  free(game->snakes);
  game->snakes = snakes;
  game->num_snakes = count;
  unsigned int index = 0;
  for (unsigned int row = 0; row < game->num_rows; row++) {
    for (unsigned int col = 0; game->board[row][col] != '\0' && game->board[row][col] != '\n'; col++) {
      if (!is_tail(game->board[row][col])) continue;
      game->snakes[index] = (snake_t){.tail_row = row, .tail_col = col, .live = true};
      find_head(game, index++);
    }
  }
  return game;
}
