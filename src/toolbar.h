#ifndef SB_TOOLBAR_H
#define SB_TOOLBAR_H

#include <SDL2/SDL.h>
#include <cairo/cairo.h>
#include <stdbool.h>

typedef enum Color {
  COLOR_PRIMARY,
  COLOR_SECONDARY,
  COLORS_AMOUNT,
} Color;

typedef enum StrokeWidth {
  STROKE_WIDTH_THIN,
  STROKE_WIDTH_MEDIUM,
  STROKE_WIDTH_THICK,
  STROKES_AMOUNT,
} StrokeWidth;

unsigned int get_color(Color color);
double get_width(StrokeWidth width);

typedef struct ToolBar {
  cairo_surface_t *cr_surface;
  cairo_t *cr;
  SDL_PixelFormat *pixel_format;

  int width;
  int height;
  int pitch;
  bool visible;

  int selected_width;
  int selected_color;
} ToolBar;

ToolBar *toolbar_create(int height, SDL_PixelFormat *pixel_format);
void toolbar_free(ToolBar *toolbar);
void toolbar_render(ToolBar *toolbar);
void toolbar_select_button(ToolBar *toolbar, int x, int *stroke_width, int *color);
#endif
