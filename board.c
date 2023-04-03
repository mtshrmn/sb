#include "board.h"

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

  cairo_surface_t *cr_surface = cairo_image_surface_create_for_data(
      (unsigned char *)sdl_surface->pixels, CAIRO_FORMAT_RGB24, sdl_surface->w, sdl_surface->h, sdl_surface->pitch);

  if (cr_surface == NULL)
    goto defer;

  int x_multiplier = renderer_width / window_width;
  int y_multiplier = renderer_height / window_height;
  cairo_surface_set_device_scale(cr_surface, x_multiplier, y_multiplier);

  cairo_t *canvas = cairo_create(cr_surface);

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  SDL_RenderClear(renderer);

  if (canvas == NULL)
    goto defer;

  Vector *current_stroke_points = vector_create(VECTOR_POINTS);
  if (current_stroke_points == NULL)
    goto defer;
  Vector *current_stroke_paths = vector_create(VECTOR_PATHS);
  if (current_stroke_paths == NULL)
    goto defer;
  Vector *strokes = vector_create(VECTOR_PATHS);
  if (strokes == NULL)
    goto defer;

  board->window = window;
  board->renderer = renderer;
  board->sdl_surface = sdl_surface;
  board->sdl_texture = sdl_texture;
  board->cr_surface = cr_surface;
  board->cr = canvas;
  board->width = window_width;
  board->height = window_height;
  board->current_stroke_points = current_stroke_points;
  board->current_stroke_paths = current_stroke_paths;
  board->strokes = strokes;
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
  return NULL;
}

void board_free(Board *board) {
  vector_free(board->strokes);
  vector_free(board->current_stroke_paths);
  vector_free(board->current_stroke_points);

  cairo_destroy(board->cr);
  cairo_surface_destroy(board->cr_surface);
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

  int cairo_x_multiplier = renderer_width / board->width;
  int cairo_y_multiplier = renderer_height / board->height;

  cairo_t *canvas = cairo_create(board->cr_surface);

  if (board->cr != canvas) {
    cairo_destroy(board->cr);
    board->cr = canvas;
  }

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
  int pitch = board->sdl_surface->pitch;
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
  cairo_set_source_rgba(board->cr, 0, 0, 0, 1.0);
  cairo_set_line_width(board->cr, LINE_WIDTH);
  cairo_set_line_cap(board->cr, CAIRO_LINE_CAP_ROUND);
  cairo_set_line_join(board->cr, CAIRO_LINE_JOIN_ROUND);
}

void board_draw_strokes(Board *board) {
  board_setup_draw(board);
  for (int i = 0; i < board->strokes->length; ++i) {
    cairo_path_t *path = vector_get(board->strokes, i);
    cairo_append_path(board->cr, path);
  }
  cairo_stroke(board->cr);
}
