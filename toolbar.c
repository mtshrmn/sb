#include "toolbar.h"
#include <SDL2/SDL.h>

unsigned int get_color(Color color) {
  // color definitions (0xAARRGGBB)
  if (color == COLOR_RED)
    return 0xFFFF0000;
  if (color == COLOR_BLACK)
    return 0xFF000000;

  // return transparent black as default
  return 0x00000000;
}

double get_width(StrokeWidth width) {
  if (width == STROKE_WIDTH_THIN)
    return 1.5;
  if (width == STROKE_WIDTH_MEDIUM)
    return 3.0;
  if (width == STROKE_WIDTH_THICK)
    return 6.0;

  // return invalid width as default
  return -1;
}

ToolBar *toolbar_create(int height, SDL_PixelFormat *pixel_format) {
  int width = height * (STROKES_AMOUNT + COLORS_AMOUNT);
  ToolBar *toolbar = malloc(sizeof(ToolBar));
  if (toolbar == NULL)
    goto defer;

  cairo_surface_t *cr_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
  if (cr_surface == NULL)
    goto defer;

  cairo_t *cr = cairo_create(cr_surface);
  if (cr == NULL)
    goto defer;

  toolbar->cr_surface = cr_surface;
  toolbar->cr = cr;
  toolbar->width = width;
  toolbar->height = height;
  toolbar->pitch = width * 4;
  toolbar->visible = true;
  toolbar->pixel_format = pixel_format;
  toolbar->selected_color = COLOR_BLACK;
  toolbar->selected_width = STROKE_WIDTH_MEDIUM;
  return toolbar;

defer:
  if (toolbar != NULL)
    free(toolbar);
  if (cr_surface != NULL)
    cairo_surface_destroy(cr_surface);
  if (cr != NULL)
    cairo_destroy(cr);

  return NULL;
}

void toolbar_free(ToolBar *toolbar) {
  cairo_destroy(toolbar->cr);
  cairo_surface_destroy(toolbar->cr_surface);
  free(toolbar);
}

void toolbar_render(ToolBar *toolbar) {
  // clear toolbar
  cairo_set_source_rgba(toolbar->cr, 1, 1, 1, 1);
  cairo_paint(toolbar->cr);
  cairo_fill(toolbar->cr);

  // draw border
  cairo_set_line_width(toolbar->cr, 2);
  cairo_set_source_rgba(toolbar->cr, 0, 0, 0, 1);
  cairo_rectangle(toolbar->cr, 0, 0, toolbar->width, toolbar->height + 1);
  cairo_stroke(toolbar->cr);

  double center_y = toolbar->height / 2.0;
  cairo_set_line_cap(toolbar->cr, CAIRO_LINE_CAP_ROUND);
  cairo_set_line_join(toolbar->cr, CAIRO_LINE_JOIN_ROUND);

  // draw stroke width
  cairo_set_source_rgba(toolbar->cr, 0, 0, 0, 1);

  for (int width = 0; width < STROKES_AMOUNT; ++width) {
    int pos = width;
    double center_x = toolbar->height * (pos + 0.5);
    cairo_move_to(toolbar->cr, center_x, center_y);
    cairo_arc(toolbar->cr, center_x, center_y, 0, 0, M_PI * 2);
    cairo_set_line_width(toolbar->cr, get_width(width));
    cairo_stroke(toolbar->cr);
  }

  // draw separator between stroke widths and colors
  cairo_set_line_width(toolbar->cr, 2);
  cairo_move_to(toolbar->cr, toolbar->height * STROKES_AMOUNT, 0);
  cairo_line_to(toolbar->cr, toolbar->height * STROKES_AMOUNT, toolbar->height);
  cairo_stroke(toolbar->cr);

  // draw colors
  cairo_set_line_width(toolbar->cr, toolbar->height / 2.0);

  for (int color = 0; color < COLORS_AMOUNT; ++color) {
    int pos = color + STROKES_AMOUNT;
    double center_x = toolbar->height * (pos + 0.5);
    cairo_move_to(toolbar->cr, center_x, center_y);
    cairo_arc(toolbar->cr, center_x, center_y, 0, 0, M_PI * 2);

    Uint8 r, g, b, a;
    SDL_GetRGBA(get_color(color), toolbar->pixel_format, &r, &g, &b, &a);
    cairo_set_source_rgba(toolbar->cr, r / 255.0, g / 255.0, b / 255.0, 1);
    cairo_stroke(toolbar->cr);
  }

  // underline selected color and stroke width
  cairo_set_line_width(toolbar->cr, 2);
  cairo_set_source_rgba(toolbar->cr, 0, 0, 0, 1);
  cairo_set_line_cap(toolbar->cr, CAIRO_LINE_CAP_SQUARE);
  double fifth = toolbar->height / 5.0;

  cairo_move_to(toolbar->cr, toolbar->selected_width * toolbar->height + fifth, fifth * 4.5);
  cairo_line_to(toolbar->cr, toolbar->selected_width * toolbar->height + fifth * 4, fifth * 4.5);
  cairo_stroke(toolbar->cr);

  cairo_move_to(toolbar->cr, (toolbar->selected_color + STROKES_AMOUNT) * toolbar->height + fifth, fifth * 4.5);
  cairo_line_to(toolbar->cr, (toolbar->selected_color + STROKES_AMOUNT) * toolbar->height + fifth * 4, fifth * 4.5);
  cairo_stroke(toolbar->cr);
}

void toolbar_select_button(ToolBar *toolbar, int x, int *stroke_width, int *color) {
  *stroke_width = -1;
  *color = -1;

  int interval = toolbar->height;
  for (int i = 0; i < STROKES_AMOUNT; ++i) {
    if (x <= interval * (i + 1)) {
      *stroke_width = i;
      toolbar->selected_width = i;
      return;
    }
  }

  for (int i = STROKES_AMOUNT; i < STROKES_AMOUNT + COLORS_AMOUNT; ++i) {
    if (x <= interval * (i + 1)) {
      *color = i - STROKES_AMOUNT;
      toolbar->selected_color = i - STROKES_AMOUNT;
      return;
    }
  }
}
