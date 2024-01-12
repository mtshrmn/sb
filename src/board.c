#include "board.h"
#include "config.h"
#include "path.h"

Board *board_create(int width, int height) {
  Board *board = malloc(sizeof(Board));
  if (board == NULL)
    goto defer;

  SDL_Window *window = SDL_CreateWindow("Simple Board", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height,
                                        SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);

  if (window == NULL)
    goto defer;

  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  if (renderer == NULL)
    goto defer;

  int window_width;
  int window_height;
  SDL_GetWindowSize(window, &window_width, &window_height);

  int renderer_width;
  int renderer_height;
  SDL_GetRendererOutputSize(renderer, &renderer_width, &renderer_height);

  SDL_Surface *sdl_surface =
      SDL_CreateRGBSurface(0, renderer_width, renderer_height, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0);

  if (sdl_surface == NULL)
    goto defer;

  SDL_Texture *sdl_texture = SDL_CreateTextureFromSurface(renderer, sdl_surface);

  if (sdl_texture == NULL)
    goto defer;

  cairo_surface_t *cr_surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, sdl_surface->w, sdl_surface->h);

  if (cr_surface == NULL)
    goto defer;

  int x_multiplier = renderer_width / window_width;
  int y_multiplier = renderer_height / window_height;
  cairo_surface_set_device_scale(cr_surface, x_multiplier, y_multiplier);

  cairo_t *canvas = cairo_create(cr_surface);

  SDL_SetRenderDrawColor(renderer, BOARD_BG);
  SDL_RenderClear(renderer);

  if (canvas == NULL)
    goto defer;

  SDL_Cursor *default_cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
  if (default_cursor == NULL)
    goto defer;

  Vector *current_stroke_points = vector_create(VECTOR_POINTS);
  if (current_stroke_points == NULL)
    goto defer;
  Vector *current_stroke_paths = vector_create(VECTOR_PATHS);
  if (current_stroke_paths == NULL)
    goto defer;
  Vector *strokes = vector_create(VECTOR_COLORED_PATHS);
  if (strokes == NULL)
    goto defer;

#ifdef USE_TOOLBAR
  ToolBar *toolbar = toolbar_create(50, sdl_surface->format);
  if (toolbar == NULL)
    goto defer;
#endif

  board->window = window;
  board->renderer = renderer;
  board->sdl_surface = sdl_surface;
  board->cursor = NULL;
  board->default_cursor = default_cursor;
  board->sdl_texture = sdl_texture;
  board->cr_surface = cr_surface;
  board->cr = canvas;
  board->width = window_width;
  board->height = window_height;
#ifdef USE_TOOLBAR
  board->toolbar = toolbar;
#endif
  board->current_stroke_points = current_stroke_points;
  board->current_stroke_paths = current_stroke_paths;
  board->strokes = strokes;
  board->dx = 0;
  board->dy = 0;
#ifdef USE_TOOLBAR
  board->stroke_width = get_width(toolbar->selected_width);
  board->stroke_color = get_color(toolbar->selected_color);
#else
  board->stroke_width = get_width(STROKE_WIDTH_MEDIUM);
  board->stroke_color = get_color(COLOR_PRIMARY);
#endif
  board->mouse_x = 0;
  board->mouse_x_raw = 0;
  board->mouse_y = 0;
  board->mouse_y_raw = 0;
  board->state = STATE_IDLE;
  return board;

defer:
  if (board != NULL)
    free(board);
  if (canvas != NULL)
    cairo_destroy(canvas);
  if (cr_surface != NULL)
    cairo_surface_destroy(cr_surface);
  if (sdl_texture != NULL)
    SDL_DestroyTexture(sdl_texture);
  if (sdl_surface != NULL)
    SDL_FreeSurface(sdl_surface);
  if (renderer != NULL)
    SDL_DestroyRenderer(renderer);
  if (window != NULL)
    SDL_DestroyWindow(window);
  if (current_stroke_points != NULL)
    vector_free(current_stroke_points);
  if (current_stroke_paths != NULL)
    vector_free(current_stroke_paths);
  if (strokes != NULL)
    vector_free(strokes);
#ifdef USE_TOOLBAR
  if (toolbar != NULL)
    toolbar_free(toolbar);
#endif
  if (default_cursor != NULL)
    SDL_FreeCursor(default_cursor);
  return NULL;
}

void board_free(Board *board) {
  vector_free(board->strokes);
  vector_free(board->current_stroke_paths);
  vector_free(board->current_stroke_points);

  cairo_destroy(board->cr);
  cairo_surface_destroy(board->cr_surface);
  SDL_FreeCursor(board->cursor);
  SDL_FreeCursor(board->default_cursor);
  SDL_DestroyTexture(board->sdl_texture);
  SDL_FreeSurface(board->sdl_surface);
  SDL_DestroyRenderer(board->renderer);
  SDL_DestroyWindow(board->window);

#ifdef USE_TOOLBAR
  toolbar_free(board->toolbar);
#endif
  free(board);
}

void board_resize_surface(Board *board) {
  SDL_GetWindowSize(board->window, &board->width, &board->height);
#ifdef USE_TOOLBAR
  board_update_toolbar_area(board);
#endif

  int renderer_width;
  int renderer_height;
  SDL_GetRendererOutputSize(board->renderer, &renderer_width, &renderer_height);

  SDL_Surface *sdl_surface =
      SDL_CreateRGBSurface(0, renderer_width, renderer_height, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0);
  if (sdl_surface == NULL) {
    return;
  }

  SDL_FreeSurface(board->sdl_surface);
  board->sdl_surface = sdl_surface;

  SDL_Texture *sdl_texture = SDL_CreateTextureFromSurface(board->renderer, sdl_surface);

  if (sdl_texture == NULL) {
    SDL_FreeSurface(sdl_surface);
  }

  SDL_DestroyTexture(board->sdl_texture);
  board->sdl_texture = sdl_texture;

  cairo_surface_t *cr_surface = cairo_image_surface_create_for_data(
      (unsigned char *)sdl_surface->pixels, CAIRO_FORMAT_RGB24, sdl_surface->w, sdl_surface->h, sdl_surface->pitch);

  if (board->cr_surface != cr_surface) {
    cairo_surface_destroy(board->cr_surface);
    board->cr_surface = cr_surface;
  }

  cairo_t *canvas = cairo_create(board->cr_surface);

  if (board->cr != canvas) {
    cairo_destroy(board->cr);
    board->cr = canvas;
  }

  // cairo canvas (cr) is recreated so we need to update its translation.
  cairo_translate(board->cr, board->dx, board->dy);
  cairo_set_source_surface(board->cr, cr_surface, 0, 0);
  cairo_paint(board->cr);
}

void board_clear(Board *board) {
  cairo_set_source_rgba(board->cr, BOARD_BG);
  cairo_paint(board->cr);
  cairo_fill(board->cr);
}

void board_render(Board *board, SDL_Rect *update_area) {
  unsigned char *data = cairo_image_surface_get_data(board->cr_surface);
  if (update_area != NULL) {
    // transform the data and "offset" it in such way
    // that when we update the surface, it appears as
    // if it is rendered in the right place.
    unsigned char *new_data = malloc(board->width * board->height * 4);
    int line = board->width * 4;
    for (int l = 0; l < board->height; ++l) {
      for (int i = l * line; i < update_area->w * 4 + l * line; ++i) {
        int transformation = i + update_area->x * 4 + update_area->y * line;
        if (transformation < board->width * board->height * 4) {
          new_data[i] = data[transformation];
        }
      }
    }
    data = new_data;
  }
  SDL_UpdateTexture(board->sdl_texture, update_area, data, board->sdl_surface->pitch);

#ifdef USE_TOOLBAR
  SDL_Rect result;
  bool is_intersecting = SDL_IntersectRect(update_area, &board->toolbar_area, &result) == SDL_TRUE;
  if (board->toolbar->visible && (is_intersecting || update_area == NULL)) {
    toolbar_render(board->toolbar);
    // draw toolbar on texture
    unsigned char *toolbar_data = cairo_image_surface_get_data(board->toolbar->cr_surface);
    SDL_UpdateTexture(board->sdl_texture, &board->toolbar_area, toolbar_data, board->toolbar->pitch);
  }
#endif
  SDL_RenderClear(board->renderer);
  SDL_RenderCopy(board->renderer, board->sdl_texture, NULL, NULL);
  SDL_RenderPresent(board->renderer);

  if (update_area != NULL) {
    free(data);
  }
}

void board_setup_draw(Board *board) {
  Uint8 r, g, b, a;
  SDL_GetRGBA(board->stroke_color, board->sdl_surface->format, &r, &g, &b, &a);
  cairo_set_source_rgba(board->cr, r / 255.0, g / 255.0, b / 255.0, a / 255.0);
  cairo_set_line_width(board->cr, board->stroke_width);
  cairo_set_line_cap(board->cr, CAIRO_LINE_CAP_ROUND);
  cairo_set_line_join(board->cr, CAIRO_LINE_JOIN_ROUND);
}

void board_draw_strokes(Board *board) {
  board_setup_draw(board);
  for (size_t i = 0; i < board->strokes->length; ++i) {
    cairo_new_path(board->cr);
    Path *path = vector_get(board->strokes, i);
    Uint8 r, g, b, a;
    SDL_GetRGBA(path->color, board->sdl_surface->format, &r, &g, &b, &a);
    cairo_set_source_rgba(board->cr, r / 255.0, g / 255.0, b / 255.0, a / 255.0);
    cairo_set_line_width(board->cr, path->width);
    cairo_append_path(board->cr, path->path);
    cairo_stroke(board->cr);
  }
}

void board_translate(Board *board, double dx, double dy) {
  if (dx == 0 && dy == 0) {
    return;
  }
  board->dx += dx;
  board->dy += dy;
  cairo_translate(board->cr, dx, dy);
  board_refresh(board);
}

void board_reset_translation(Board *board) {
  board_translate(board, -board->dx, -board->dy);
}

void board_refresh(Board *board) {
  board_clear(board);
  board_draw_strokes(board);
  board_render(board, NULL);
}

void board_update_mouse_state(Board *board) {
  SDL_GetMouseState(&board->mouse_x_raw, &board->mouse_y_raw);
  board->mouse_x = board->mouse_x_raw - board->dx;
  board->mouse_y = board->mouse_y_raw - board->dy;
}

void board_update_cursor(Board *board) {
  double width = board->stroke_width;
  SDL_Surface *cursor_surface = SDL_CreateRGBSurfaceWithFormat(0, width * 2, width * 2, 32, SDL_PIXELFORMAT_RGBA32);
  if (cursor_surface == NULL) {
    return;
  }

  // motivation:
  // 1. create a cairo instance
  // 2. draw an anti aliased circle
  // 3. copy it to the cursor surface
  // 4. delete the cairo instance.
  double w = cursor_surface->w;
  double h = cursor_surface->h;
  cairo_surface_t *cr_surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, w, h);
  cairo_t *cr = cairo_create(cr_surface);

  Uint8 r, g, b, a;
  SDL_GetRGBA(board->stroke_color, cursor_surface->format, &r, &g, &b, &a);
  cairo_set_source_rgba(cr, r / 255.0, g / 255.0, b / 255.0, a / 255.0);

  cairo_set_line_width(cr, width);
  cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
  cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);

  // move cairo to the center of the surface.
  cairo_move_to(cr, w / 2, h / 2);
  cairo_arc(cr, w / 2, h / 2, 0, 0, M_PI * 2);
  cairo_stroke(cr);

  // render to sdl surface
  unsigned char *data = cairo_image_surface_get_data(cr_surface);
  Uint32 *cursor_surface_pixels = cursor_surface->pixels;
  memcpy(cursor_surface_pixels, data, w * h * 4);

  SDL_Cursor *new_cursor = SDL_CreateColorCursor(cursor_surface, w / 2, h / 2);
  if (new_cursor != NULL) {
    SDL_Cursor *temp_cursor = board->cursor;
    board->cursor = new_cursor;
    SDL_SetCursor(board->cursor);
    SDL_FreeCursor(temp_cursor);
  }

  cairo_destroy(cr);
  cairo_surface_destroy(cr_surface);
  SDL_FreeSurface(cursor_surface);
}

#ifdef USE_TOOLBAR
void board_update_toolbar_area(Board *board) {
  SDL_Rect toolbar_area = {
      .x = (board->width - board->toolbar->width) / 2,
      .y = board->height - board->toolbar->height,
      .w = board->toolbar->width,
      .h = board->toolbar->height,
  };
  board->toolbar_area = toolbar_area;
}

void board_click_toolbar(Board *board, double x) {
  int color, width;
  toolbar_select_button(board->toolbar, x - board->toolbar_area.x, &width, &color);
  if (width >= 0) {
    board->stroke_width = get_width(width);
  }

  if (color >= 0) {
    board->stroke_color = get_color(color);
  }

  board_update_cursor(board);
  board_refresh(board);
}
#endif

void board_reset_current_stroke(Board *board) {
  vector_reset(board->current_stroke_points);
  vector_reset(board->current_stroke_paths);
}

void board_set_stroke_width(Board *board, StrokeWidth width) {
#ifdef USE_TOOLBAR
  board->toolbar->selected_width = width;
#endif
  board->stroke_width = get_width(width);
  board_update_cursor(board);
  board_refresh(board);
}

void board_set_stroke_color(Board *board, Color color) {
#ifdef USE_TOOLBAR
  board->toolbar->selected_color = color;
#endif
  board->stroke_color = get_color(color);
  board_update_cursor(board);
  board_refresh(board);
}
