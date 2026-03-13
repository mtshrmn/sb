#include "board.h"
#include "config.h"
#include "path.h"
#include "point.h"

#define DEFER_IF_NULL(x)                                                                                               \
  do {                                                                                                                 \
    if ((x) == NULL) {                                                                                                 \
      goto defer;                                                                                                      \
    }                                                                                                                  \
  } while (0)

Board *board_create(int width, int height) {
  Board *board = malloc(sizeof(Board));
  SDL_Window *window = NULL;
  SDL_Renderer *renderer = NULL;
  SDL_Surface *sdl_surface = NULL;
  SDL_Texture *sdl_texture = NULL;
  cairo_surface_t *cr_surface = NULL;
  cairo_t *canvas = NULL;
  SDL_Cursor *default_cursor = NULL;
  List *current_stroke_points = NULL;
  List *current_stroke_paths = NULL;
  pdll *strokes = NULL;

  DEFER_IF_NULL(board);

  window = SDL_CreateWindow("Simple Board", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height,
                            SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);
  DEFER_IF_NULL(window);

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  DEFER_IF_NULL(renderer);

  int window_width;
  int window_height;
  SDL_GetWindowSize(window, &window_width, &window_height);

  int renderer_width;
  int renderer_height;
  SDL_GetRendererOutputSize(renderer, &renderer_width, &renderer_height);

  sdl_surface = SDL_CreateRGBSurface(0, renderer_width, renderer_height, 32, R_MASK, G_MASK, B_MASK, 0);
  DEFER_IF_NULL(sdl_surface);

  sdl_texture = SDL_CreateTextureFromSurface(renderer, sdl_surface);
  DEFER_IF_NULL(sdl_texture);

  cr_surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, sdl_surface->w, sdl_surface->h);
  DEFER_IF_NULL(cr_surface);

  int x_multiplier = renderer_width / window_width;
  int y_multiplier = renderer_height / window_height;
  cairo_surface_set_device_scale(cr_surface, x_multiplier, y_multiplier);

  canvas = cairo_create(cr_surface);

  SDL_SetRenderDrawColor(renderer, BOARD_BG_CAIRO);
  SDL_RenderClear(renderer);
  DEFER_IF_NULL(canvas);

  default_cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
  DEFER_IF_NULL(default_cursor);

  current_stroke_points = list_create((list_free_function)point_free);
  DEFER_IF_NULL(current_stroke_points);
  current_stroke_paths = list_create((list_free_function)cairo_path_destroy);
  DEFER_IF_NULL(current_stroke_paths);
  strokes = pdll_init((pdll_free_node_data_func)path_free);
  DEFER_IF_NULL(strokes);

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
  board->current_stroke_points = current_stroke_points;
  board->current_stroke_paths = current_stroke_paths;
  board->strokes = strokes;
  board->dx = 0;
  board->dy = 0;
  board->stroke_width = STROKE_WIDTH_MEDIUM;
  board->stroke_color = COLOR_PRIMARY;
  board->stroke_width_previous = board->stroke_width;
  board->mouse_x = 0;
  board->mouse_x_raw = 0;
  board->mouse_y = 0;
  board->mouse_y_raw = 0;
  board->state = STATE_IDLE;
  return board;

defer:
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
    list_free(current_stroke_points);
  if (current_stroke_paths != NULL)
    list_free(current_stroke_paths);
  if (strokes != NULL)
    pdll_free(strokes);
  if (default_cursor != NULL)
    SDL_FreeCursor(default_cursor);
  return NULL;
}

void board_free(Board *board) {
  pdll_free(board->strokes);
  list_free(board->current_stroke_paths);
  list_free(board->current_stroke_points);

  cairo_destroy(board->cr);
  cairo_surface_destroy(board->cr_surface);
  SDL_FreeCursor(board->cursor);
  SDL_FreeCursor(board->default_cursor);
  SDL_DestroyTexture(board->sdl_texture);
  SDL_FreeSurface(board->sdl_surface);
  SDL_DestroyRenderer(board->renderer);
  SDL_DestroyWindow(board->window);

  free(board);
}

void board_resize_surface(Board *board) {
  SDL_GetWindowSize(board->window, &board->width, &board->height);

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
  cairo_set_source_rgba(board->cr, BOARD_BG_CAIRO);
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
  pdll_iter(board->strokes, node) {
    cairo_new_path(board->cr);
    Path *path = node->data;
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

  // if stroke_color is the same as the background color
  // add an outline to the cursor
  if (board->stroke_color == BOARD_BG) {
    Uint8 r, g, b, a;
    SDL_GetRGBA(BOARD_BG_INVERTED, cursor_surface->format, &r, &g, &b, &a);
    cairo_set_source_rgba(cr, r / 255.0, g / 255.0, b / 255.0, a / 255.0);

    cairo_set_line_width(cr, 2);
    cairo_arc(cr, w / 2, h / 2, width / 2, 0, M_PI * 2);
    cairo_stroke(cr);
  }

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

void board_reset_current_stroke(Board *board) {
  list_reset(board->current_stroke_points);
  list_reset(board->current_stroke_paths);
}

void board_set_stroke_width(Board *board, double width) {
  board->stroke_width = width;
  board_update_cursor(board);
  board_refresh(board);
}

void board_set_stroke_color(Board *board, unsigned int color) {
  board->stroke_color = color;
  board_update_cursor(board);
  board_refresh(board);
}

ScratchPad *scratchpad_new(cairo_path_t *query) {
  // synopsis:
  // use cairo's builtin algorithms to check if two paths are intersecting.
  // this is done in two steps
  // 1. create a scratch pad with the eraser path.
  // 2. using that scratch pad to check if a path intersects with the eraser.
  //
  // motivation:
  // create 2 surfaces (and cairo contexts):
  // one will be the "query" - stores the eraser path
  // the other one will be the "scratch" - we copy the query surface with
  // a mask - if both paths intersect we actually draw the eraser pixel on that surface
  // meaning that if as a result we get a non-zero pixel - both paths intersect.
  //
  // since those surfaces are WAY smaller than the infinite board,
  // we scale everything down by the bounding box of the eraser path
  // and translate it to be in the center of the screen.
  // this function - "scratchpad_new()" just sets both surfaces and related
  // information that is needed later for the intersection check

  // create a temp surface just to compute the needed transformation and translation.
  cairo_surface_t *tmp_surface = cairo_image_surface_create(CAIRO_FORMAT_A8, 1, 1);
  if (tmp_surface == NULL)
    return NULL;

  cairo_t *tmp_cr = cairo_create(tmp_surface);
  if (tmp_cr == NULL) {
    cairo_surface_destroy(tmp_surface);
    return NULL;
  }

  double x1, y1, x2, y2;
  cairo_append_path(tmp_cr, query);
  cairo_path_extents(tmp_cr, &x1, &y1, &x2, &y2);
  cairo_destroy(tmp_cr);
  cairo_surface_destroy(tmp_surface);

  // since using cairo_path_extents() is less expensive than cairo_stroke_extents()
  // we use it but then we need to take into account for the thickness of the stroke.
  // extend the bounding box by the stroke thickness in every direction.
  double origin_x = x1 - STROKE_WIDTH_THICKEST;
  double origin_y = y1 - STROKE_WIDTH_THICKEST;
  double scale_x = SCRATCH_PAD_WIDTH / (x2 - x1 + 2 * STROKE_WIDTH_THICKEST);
  double scale_y = SCRATCH_PAD_HEIGHT / (y2 - y1 + 2 * STROKE_WIDTH_THICKEST);

  // create both surfaces and cairo contexts.
  cairo_surface_t *query_surface = cairo_image_surface_create(CAIRO_FORMAT_A8, SCRATCH_PAD_WIDTH, SCRATCH_PAD_HEIGHT);
  if (query_surface == NULL)
    return NULL;

  cairo_surface_t *scratch_surface = cairo_image_surface_create(CAIRO_FORMAT_A8, SCRATCH_PAD_WIDTH, SCRATCH_PAD_HEIGHT);
  if (scratch_surface == NULL) {
    cairo_surface_destroy(query_surface);
    return NULL;
  }

  cairo_t *query_cr = cairo_create(query_surface);
  if (query_cr == NULL) {
    cairo_surface_destroy(query_surface);
    cairo_surface_destroy(scratch_surface);
    return NULL;
  }

  cairo_t *scratch_cr = cairo_create(scratch_surface);
  if (scratch_cr == NULL) {
    cairo_surface_destroy(query_surface);
    cairo_surface_destroy(scratch_surface);
    cairo_destroy(query_cr);
    return NULL;
  }

  // render eraser path into query_surface and populate the scratchpad
  cairo_set_line_width(query_cr, STROKE_WIDTH_THICKEST);
  cairo_set_line_cap(query_cr, CAIRO_LINE_CAP_ROUND);
  cairo_set_line_join(query_cr, CAIRO_LINE_JOIN_ROUND);
  cairo_scale(query_cr, scale_x, scale_y);
  cairo_translate(query_cr, -origin_x, -origin_y);
  cairo_append_path(query_cr, query);
  cairo_stroke(query_cr);

  ScratchPad *pad = malloc(sizeof(ScratchPad));
  if (pad == NULL) {
    cairo_surface_destroy(query_surface);
    cairo_surface_destroy(scratch_surface);
    cairo_destroy(query_cr);
    cairo_destroy(scratch_cr);
    return NULL;
  }

  pad->scratch_cr = scratch_cr;
  pad->query_cr = query_cr;
  pad->scratch_surface = scratch_surface;
  pad->query_surface = query_surface;
  pad->scale_x = scale_x;
  pad->scale_y = scale_y;
  pad->origin_x = origin_x;
  pad->origin_y = origin_y;
  return pad;
}

void scratchpad_destroy(ScratchPad *pad) {
  cairo_surface_destroy(pad->query_surface);
  cairo_surface_destroy(pad->scratch_surface);
  cairo_destroy(pad->scratch_cr);
  cairo_destroy(pad->query_cr);
  free(pad);
}

bool scratchpad_test_intersection(ScratchPad *pad, cairo_path_t *path, double stroke_width) {
  // this is the second part of the intersection checking.
  // copy the rendered eraser path from the query_surface.
  cairo_set_operator(pad->scratch_cr, CAIRO_OPERATOR_SOURCE);
  cairo_set_source_surface(pad->scratch_cr, pad->query_surface, 0, 0);
  cairo_paint(pad->scratch_cr);

  // draw the path on top using OPERATOR_IN:
  // only pixels where both the eraser and the path are painted survive.
  cairo_save(pad->scratch_cr);
  cairo_set_line_width(pad->scratch_cr, stroke_width);
  cairo_set_line_cap(pad->scratch_cr, CAIRO_LINE_CAP_ROUND);
  cairo_set_line_join(pad->scratch_cr, CAIRO_LINE_JOIN_ROUND);
  cairo_scale(pad->scratch_cr, pad->scale_x, pad->scale_y);
  cairo_translate(pad->scratch_cr, -pad->origin_x, -pad->origin_y);
  cairo_append_path(pad->scratch_cr, path);
  cairo_set_operator(pad->scratch_cr, CAIRO_OPERATOR_IN);
  cairo_stroke(pad->scratch_cr);
  cairo_restore(pad->scratch_cr);

  cairo_surface_flush(pad->scratch_surface);
  const uint8_t *data = cairo_image_surface_get_data(pad->scratch_surface);
  int stride = cairo_image_surface_get_stride(pad->scratch_surface);

  if (stride == SCRATCH_PAD_WIDTH) {
    return memchr(data, 0xFF, SCRATCH_PAD_WIDTH * SCRATCH_PAD_HEIGHT) != NULL;
  }

  for (int y = 0; y < SCRATCH_PAD_HEIGHT; ++y) {
    if (memchr(data + y * stride, 0xFF, SCRATCH_PAD_WIDTH))
      return true;
  }

  return false;
}

int board_delete_intersecting_paths(Board *board, cairo_path_t *path) {
  int did_paths_got_deleted = 0;
  ScratchPad *pad = scratchpad_new(path);
  if (pad == NULL) {
    return false;
  }

  pdll_iter(board->strokes, node) {
    Path *path = node->data;
    if (scratchpad_test_intersection(pad, path->path, path->width)) {
      did_paths_got_deleted = 1;
      pdll_node_mark_for_deletion(node);
    }
  }

  if (did_paths_got_deleted) {
    pdll_delete_marked_nodes(board->strokes);
  }

  scratchpad_destroy(pad);
  board_refresh(board);
  return did_paths_got_deleted;
}

int board_save_image(Board *board, char *path) {
  // calculate bounding area of image
  Point top_left, bottom_right;
  cairo_path_t *prev_path = cairo_copy_path(board->cr);

  pdll_iter(board->strokes, node) {
    cairo_path_t *path = ((Path *)node->data)->path;
    cairo_append_path(board->cr, path);
  }

  cairo_path_extents(board->cr, &top_left.x, &top_left.y, &bottom_right.x, &bottom_right.y);

  // restore old path
  cairo_new_path(board->cr);
  cairo_append_path(board->cr, prev_path);
  cairo_path_destroy(prev_path);

  int width = bottom_right.x - top_left.x + 10;
  int height = bottom_right.y - top_left.y + 10;
  cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, width, height);
  cairo_t *cr = cairo_create(surface);

  // draw strokes on newly created cairo surface
  cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
  cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
  cairo_translate(cr, -top_left.x + 5, -top_left.y + 5);

  pdll_iter(board->strokes, node) {
    Path *path = node->data;
    Uint8 r, g, b, a;
    SDL_GetRGBA(path->color, board->sdl_surface->format, &r, &g, &b, &a);
    cairo_set_source_rgba(cr, r / 255.0, g / 255.0, b / 255.0, a / 255.0);
    cairo_set_line_width(cr, path->width);
    cairo_append_path(cr, path->path);
    cairo_stroke(cr);
  }

  cairo_surface_write_to_png(surface, path);
  cairo_destroy(cr);
  cairo_surface_destroy(surface);
  return 0;
}
