#ifndef SB_BOARD_H
#define SB_BOARD_H

#include "vector.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <cairo/cairo.h>

// r, g, b, a
#define BOARD_BG 1, 1, 1, 1.0
#define BOARD_FG 0, 0, 0, 1.0
#define LINE_WIDTH 3.0

typedef enum BoardState {
  STATE_IDLE,
  STATE_DRAWING,
  STATE_MOVING,
} BoardState;

typedef struct Board {
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Surface *sdl_surface;
  SDL_Texture *sdl_texture;

  cairo_surface_t *cr_surface;
  cairo_t *cr;
  int width;
  int height;

  // translation of board
  double dx;
  double dy;

  Vector *current_stroke_points; // contains Point
  Vector *current_stroke_paths;  // contains cairo_path_t
  Vector *strokes;               // contains cairo_path_t
  BoardState state;
} Board;

Board *board_create(int width, int height);
void board_free(Board *board);
void board_resize_surface(Board *board);
void board_clear(Board *board);
void board_render(Board *board, SDL_Rect *update_are);
void board_setup_draw(Board *board);
void board_draw_strokes(Board *board);
void board_translate(Board *board, double dx, double dy);
void board_reset_translation(Board *board);

#endif // SB_BOARD_H
