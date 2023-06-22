#ifndef SB_BOARD_H
#define SB_BOARD_H

#include "config.h"
#include "toolbar.h"
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
  double stroke_color;

#ifdef USE_TOOLBAR
  ToolBar *toolbar;
#endif
  SDL_Rect toolbar_area;
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
#ifdef USE_TOOLBAR
void board_update_toolbar_area(Board *board);
void board_click_toolbar(Board *board, double x);
#endif
void board_reset_current_stroke(Board *board);
#endif // SB_BOARD_H
