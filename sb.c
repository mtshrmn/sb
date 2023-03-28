#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <cairo/cairo.h>

#define GROWTH_RATE 2
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define LINE_WIDTH 3.0
#define RATIO 3.0
#define TAU 6.28318530718

typedef struct {
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Surface *sdl_surface;
  SDL_Texture *sdl_texture;
  cairo_surface_t *cr_surface;
  cairo_t *canvas;
  int width;
  int height;
} Board;

Board *create_board(int width, int height) {
  Board *board = malloc(sizeof(Board));
  SDL_Window *window = SDL_CreateWindow("Simple Board", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height,
                                        SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);

  if (window == NULL) {
    free(board);
    return NULL;
  }

  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  if (renderer == NULL) {
    free(board);
    SDL_DestroyWindow(window);
    return NULL;
  }

  int window_width;
  int window_height;
  SDL_GetWindowSize(window, &window_width, &window_height);

  int renderer_width;
  int renderer_height;
  SDL_GetRendererOutputSize(renderer, &renderer_width, &renderer_height);

  SDL_Surface *sdl_surface =
      SDL_CreateRGBSurface(0, renderer_width, renderer_height, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0);

  if (sdl_surface == NULL) {
    free(board);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    return NULL;
  }

  SDL_Texture *sdl_texture = SDL_CreateTextureFromSurface(renderer, sdl_surface);

  if (sdl_texture == NULL) {
    free(board);
    SDL_FreeSurface(sdl_surface);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    return NULL;
  }

  cairo_surface_t *cr_surface = cairo_image_surface_create_for_data(
      (unsigned char *)sdl_surface->pixels, CAIRO_FORMAT_RGB24, sdl_surface->w, sdl_surface->h, sdl_surface->pitch);

  if (cr_surface == NULL) {
    free(board);
    SDL_DestroyTexture(sdl_texture);
    SDL_FreeSurface(sdl_surface);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    return NULL;
  }

  int x_multiplier = renderer_width / window_width;
  int y_multiplier = renderer_height / window_height;
  cairo_surface_set_device_scale(cr_surface, x_multiplier, y_multiplier);

  cairo_t *canvas = cairo_create(cr_surface);

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  SDL_RenderClear(renderer);

  if (canvas == NULL) {
    free(board);
    cairo_surface_destroy(cr_surface);
    SDL_DestroyTexture(sdl_texture);
    SDL_FreeSurface(sdl_surface);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    return NULL;
  }

  board->window = window;
  board->renderer = renderer;
  board->sdl_surface = sdl_surface;
  board->sdl_texture = sdl_texture;
  board->cr_surface = cr_surface;
  board->canvas = canvas;
  board->width = window_width;
  board->height = window_height;

  return board;
}

void free_board(Board *board) {
  if (board == NULL) {
    return;
  }

  cairo_destroy(board->canvas);
  cairo_surface_destroy(board->cr_surface);
  SDL_DestroyTexture(board->sdl_texture);
  SDL_FreeSurface(board->sdl_surface);
  SDL_DestroyRenderer(board->renderer);
  SDL_DestroyWindow(board->window);
  free(board);
}

void resize_board_surface(Board *board) {
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

  if (board->canvas != canvas) {
    cairo_destroy(board->canvas);
    board->canvas = canvas;
  }

  cairo_set_source_surface(board->canvas, cr_surface, 0, 0);
  cairo_paint(board->canvas);
}

void clear_board(const Board *board) {
  cairo_set_source_rgba(board->canvas, 1, 1, 1, 1.0);
  cairo_paint(board->canvas);
  cairo_fill(board->canvas);
}

void render_board(const Board *board, SDL_Rect *update_area) {
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

typedef struct {
  double x;
  double y;
} sb_point_t;

sb_point_t create_point(double x, double y) {
  sb_point_t point = {.x = x, .y = y};
  return point;
}

sb_point_t subtract_sb_points(sb_point_t p1, sb_point_t p2) {
  double x = p1.x - p2.x;
  double y = p1.y - p2.y;
  return create_point(x, y);
}

sb_point_t sum_sb_points(sb_point_t p1, sb_point_t p2) {
  double x = p1.x + p2.x;
  double y = p1.y + p2.y;
  return create_point(x, y);
}

double sb_point_length(sb_point_t p) {
  double point_length_squared = p.x * p.x + p.y * p.y;
  double point_length = sqrt(point_length_squared);
  return point_length;
}

sb_point_t multiply_sb_point(sb_point_t p, double multiplier) {
  return create_point(p.x * multiplier, p.y * multiplier);
}

sb_point_t normalize_sb_point(sb_point_t p) {
  double length = sb_point_length(p);
  if (length == 0) {
    return p;
  }
  return multiply_sb_point(p, 1.0 / length);
}

void create_handle_triple(sb_point_t origin, sb_point_t dest, sb_point_t next, sb_point_t *h1, sb_point_t *h2) {
  sb_point_t parallel_direction_next = normalize_sb_point(subtract_sb_points(next, origin));
  sb_point_t delta = subtract_sb_points(dest, origin);
  sb_point_t ratio_delta = multiply_sb_point(delta, 1.0 / RATIO);
  *h1 = sum_sb_points(origin, ratio_delta);

  double delta_length = sb_point_length(delta);
  sb_point_t handle_offset = multiply_sb_point(parallel_direction_next, delta_length / RATIO);
  *h2 = subtract_sb_points(dest, handle_offset);
}

void create_handle_quad(sb_point_t prev, sb_point_t origin, sb_point_t dest, sb_point_t next, sb_point_t *h1,
                        sb_point_t *h2) {
  sb_point_t parallel_direction_prev = normalize_sb_point(subtract_sb_points(dest, prev));
  sb_point_t parallel_direction_next = normalize_sb_point(subtract_sb_points(next, origin));
  sb_point_t delta = subtract_sb_points(dest, origin);
  double delta_len = sb_point_length(delta);
  *h1 = sum_sb_points(origin, multiply_sb_point(parallel_direction_prev, delta_len / RATIO));
  *h2 = subtract_sb_points(dest, multiply_sb_point(parallel_direction_next, delta_len / RATIO));
}

typedef struct {
  sb_point_t *points;
  int occupied;
  int capacity;
} sb_points_vec;

sb_points_vec create_sb_points_vec() {
  sb_points_vec v = {.points = NULL, .occupied = 0, .capacity = 0};
  return v;
}

void reset_sb_points_vec(sb_points_vec *vec) {
  // ...
  vec->occupied = 0;
}

void free_sb_points_vec(sb_points_vec *vec) {
  free(vec->points);
  vec->occupied = 0;
  vec->capacity = 0;
}

void sb_append_points_vec(sb_points_vec *vec, sb_point_t point) {
  if (vec->capacity == 0) {
    vec->points = malloc(sizeof(sb_point_t));
    if (vec->points == NULL) {
      return;
    }

    vec->points[0] = point;
    vec->capacity = 1;
    vec->occupied = 1;
    return;
  }
  // conditionally increase size of sub_paths
  if (vec->capacity == vec->occupied) {
    int new_size = sizeof(sb_point_t) * vec->capacity * GROWTH_RATE;
    sb_point_t *new_points = realloc(vec->points, new_size);
    if (new_points == NULL) {
      return;
    }

    vec->points = new_points;
    vec->capacity = vec->capacity * GROWTH_RATE;
  }

  vec->points[vec->occupied] = point;
  vec->occupied++;
}

typedef struct {
  cairo_path_t *sub_paths;
  int occupied;
  int capacity;
} sb_path_t;

sb_path_t create_sb_path() {
  sb_path_t path = {.sub_paths = NULL, .occupied = 0, .capacity = 0};
  return path;
}

void reset_sb_path(sb_path_t *path) {
  // ...
  path->occupied = 0;
}

void free_sb_path(sb_path_t *path) {
  for (int i = 0; i < path->occupied; ++i) {
    cairo_path_destroy(&path->sub_paths[i]);
  }
  path->occupied = 0;
  path->capacity = 0;
  free(path->sub_paths);
}

void sb_append_subpath(sb_path_t *path, cairo_path_t *sub_path) {
  if (path->capacity == 0) {
    path->sub_paths = malloc(sizeof(cairo_path_t));
    if (path->sub_paths == NULL) {
      return;
    }

    path->sub_paths[0] = *sub_path;
    path->capacity = 1;
    path->occupied = 1;
    return;
  }
  // conditionally increase size of sub_paths
  if (path->capacity == path->occupied) {
    int new_size = sizeof(cairo_path_t) * path->capacity * GROWTH_RATE;
    cairo_path_t *new_sub_paths = realloc(path->sub_paths, new_size);
    if (new_sub_paths == NULL) {
      return;
    }

    path->sub_paths = new_sub_paths;
    path->capacity = path->capacity * GROWTH_RATE;
  }

  path->sub_paths[path->occupied] = *sub_path;
  path->occupied++;
}

// keeps previous cairo path
cairo_path_t *sb_to_cairo(cairo_t *cr, sb_path_t *sb_path) {
  cairo_path_t *old_path = cairo_copy_path(cr);
  cairo_new_path(cr);
  for (int i = 0; i < sb_path->occupied; ++i) {
    cairo_append_path(cr, &sb_path->sub_paths[i]);
  }

  cairo_path_t *generated_path = cairo_copy_path(cr);
  cairo_new_path(cr);
  cairo_append_path(cr, old_path);
  return generated_path;
}

SDL_Rect get_path_bounding_area(cairo_t *cr) {
  double x1;
  double y1;
  double x2;
  double y2;
  cairo_stroke_extents(cr, &x1, &y1, &x2, &y2);
  int x = (int)(x1 - 3);
  int y = (int)(y1 - 3);
  int w = (int)(x2 - x + 3);
  int h = (int)(y2 - y + 3);
  SDL_Rect area = {.x = MAX(0, x), .y = MAX(0, y), .w = w, .h = h};
  return area;
}

int main() {
  SDL_Init(SDL_INIT_VIDEO);
  Board *board = create_board(600, 480);

  // SORT THOSE OUT:
  bool running = true;
  bool is_drawing = false;
  int mouse_x, mouse_y;
  sb_points_vec current_stroke = create_sb_points_vec();
  sb_path_t sb_path = create_sb_path();
  sb_path_t all_strokes = create_sb_path();

  while (running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT:
        running = false;
        break;
      case SDL_WINDOWEVENT: {
        if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
          resize_board_surface(board);
          clear_board(board);
          cairo_set_source_rgba(board->canvas, 0, 0, 0, 1.0);
          cairo_set_line_width(board->canvas, LINE_WIDTH);
          cairo_set_line_cap(board->canvas, CAIRO_LINE_CAP_ROUND);
          cairo_set_line_join(board->canvas, CAIRO_LINE_JOIN_ROUND);
          for (int i = 0; i < all_strokes.occupied; ++i) {
            cairo_append_path(board->canvas, &all_strokes.sub_paths[i]);
          }
          cairo_stroke(board->canvas);
          render_board(board, NULL);
        }
      } break;
      case SDL_MOUSEBUTTONDOWN: {
        is_drawing = true;
        SDL_GetMouseState(&mouse_x, &mouse_y);
        sb_point_t point = create_point(mouse_x, mouse_y);
        cairo_set_source_rgba(board->canvas, 0, 0, 0, 1.0);
        cairo_set_line_width(board->canvas, LINE_WIDTH);
        cairo_set_line_cap(board->canvas, CAIRO_LINE_CAP_ROUND);
        cairo_set_line_join(board->canvas, CAIRO_LINE_JOIN_ROUND);
        reset_sb_points_vec(&current_stroke);
        reset_sb_path(&sb_path);
        sb_append_points_vec(&current_stroke, point);
        cairo_move_to(board->canvas, point.x, point.y);
        cairo_arc(board->canvas, mouse_x, mouse_y, 0, 0, TAU);
        cairo_path_t *point_path = cairo_copy_path(board->canvas);
        sb_append_subpath(&sb_path, point_path);
        cairo_stroke(board->canvas);
        SDL_Rect bounds = {
            .x = point.x - LINE_WIDTH,
            .y = point.y - LINE_WIDTH,
            .w = 2 * LINE_WIDTH,
            .h = 2 * LINE_WIDTH,
        };
        render_board(board, &bounds);
      } break;
      case SDL_MOUSEBUTTONUP: {
        is_drawing = false;
        cairo_path_t *cr_path = sb_to_cairo(board->canvas, &sb_path);
        sb_append_subpath(&all_strokes, cr_path);
      } break;
      case SDL_MOUSEMOTION: {
        if (is_drawing == false) {
          break;
        }

        SDL_GetMouseState(&mouse_x, &mouse_y);
        sb_point_t point = create_point(mouse_x, mouse_y);
        // it is guaranteed that there's at least one point in current_stroke.points
        sb_point_t last_point = current_stroke.points[current_stroke.occupied - 1];
        if (point.x == last_point.x && point.y == last_point.y) {
          break;
        }

        sb_append_points_vec(&current_stroke, point);
        switch (current_stroke.occupied) {
        // after appending another point, there's at least 2 points
        case 2: {
          sb_point_t prev = current_stroke.points[0];
          cairo_move_to(board->canvas, prev.x, prev.y);
        } break;
        case 3: {
          sb_point_t origin = current_stroke.points[0];
          sb_point_t dest = current_stroke.points[1];
          sb_point_t next = current_stroke.points[2];

          sb_point_t h1, h2;
          create_handle_triple(origin, dest, next, &h1, &h2);
          cairo_move_to(board->canvas, origin.x, origin.y);
          cairo_curve_to(board->canvas, h1.x, h1.y, h2.x, h2.y, dest.x, dest.y);
        } break;
        default: {
          int last = current_stroke.occupied - 1;
          sb_point_t prev = current_stroke.points[last - 3];
          sb_point_t origin = current_stroke.points[last - 2];
          sb_point_t dest = current_stroke.points[last - 1];
          sb_point_t next = current_stroke.points[last];
          sb_point_t h1, h2;
          create_handle_quad(prev, origin, dest, next, &h1, &h2);
          cairo_move_to(board->canvas, origin.x, origin.y);
          cairo_curve_to(board->canvas, h1.x, h1.y, h2.x, h2.y, dest.x, dest.y);
        } break;
        }

        cairo_path_t *subpath = cairo_copy_path(board->canvas);
        sb_append_subpath(&sb_path, subpath);
        SDL_Rect bounds = get_path_bounding_area(board->canvas);
        cairo_stroke(board->canvas);
        render_board(board, &bounds);
      } break;
      case SDL_KEYDOWN: {
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        // ctrl+z
        if (keys[SDL_SCANCODE_LCTRL] && keys[SDL_SCANCODE_Z]) {
          if (all_strokes.occupied == 0) {
            break;
          }
          all_strokes.occupied--;
          clear_board(board);
          cairo_set_source_rgba(board->canvas, 0, 0, 0, 1.0);
          cairo_set_line_width(board->canvas, LINE_WIDTH);
          cairo_set_line_cap(board->canvas, CAIRO_LINE_CAP_ROUND);
          cairo_set_line_join(board->canvas, CAIRO_LINE_JOIN_ROUND);
          for (int i = 0; i < all_strokes.occupied; ++i) {
            cairo_append_path(board->canvas, &all_strokes.sub_paths[i]);
          }
          cairo_stroke(board->canvas);
          render_board(board, NULL);
        }
      } break;
      default:
        break;
      }
    }
    SDL_Delay(1000 / 60);
  }
  free_board(board);
  SDL_Quit();
}
