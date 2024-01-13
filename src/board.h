#ifndef SB_BOARD_H
#define SB_BOARD_H

#include "config.h"
#include "vector.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <cairo/cairo.h>

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
  SDL_Cursor *default_cursor;

  cairo_surface_t *cr_surface;
  cairo_t *cr;
  int width;
  int height;

  // translation of board
  double dx;
  double dy;

  double stroke_width;
  double stroke_width_previous;
  double stroke_color;
  double stroke_color_previous;

  Vector *current_stroke_points; // contains Point
  Vector *current_stroke_paths;  // contains cairo_path_t
  Vector *strokes;               // contains Path
  BoardState state;
  double mouse_x;
  int mouse_x_raw;
  double mouse_y;
  int mouse_y_raw;
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
void board_update_mouse_state(Board *board);
void board_reset_current_stroke(Board *board);
void board_set_stroke_width(Board *board, double width);
void board_set_stroke_color(Board *board, unsigned int color);
#endif // SB_BOARD_H
