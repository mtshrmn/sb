#ifndef SB_BOARD_H
#define SB_BOARD_H

#include "vector.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <cairo/cairo.h>

// r, g, b, a
#define BOARD_BG 1, 1, 1, 1.0

// color definitions (0xAARRGGBB)
#define BLACK 0xFF000000
#define RED 0xFFFF0000

#define STROKE_WIDTH_THIN 1.5
#define STROKE_WIDTH_MEDIUM 3.0
#define STROKE_WIDTH_THICK 6.0

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
  SDL_Cursor *cursor;

  cairo_surface_t *cr_surface;
  cairo_t *cr;
  int width;
  int height;

  // translation of board
  double dx;
  double dy;

  double stroke_width;
  double stroke_color;

  Vector *current_stroke_points; // contains Point
  Vector *current_stroke_paths;  // contains cairo_path_t
  Vector *strokes;               // contains Path
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
void board_refresh(Board *board);
void board_update_cursor(Board *board);
#endif // SB_BOARD_H
