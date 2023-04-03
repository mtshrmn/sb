#include "board.h"
#include "point.h"
#include "vector.h"
#include <stdbool.h>

#define TAU 6.28318530718
#define MAX(a, b) ((a) > (b) ? (a) : (b))

cairo_path_t *merge_paths(cairo_t *cr, Vector *paths) {
  // assume paths->type == VECTOR_PATHS
  cairo_new_path(cr);
  for (int i = 0; i < paths->length; ++i) {
    cairo_path_t *sub_path = vector_get(paths, i);
    cairo_append_path(cr, sub_path);
  }

  cairo_path_t *merged_path = cairo_copy_path(cr);
  return merged_path;
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
  Board *board = board_create(600, 480);
  bool running = true;
  bool is_drawing = false;
  int mouse_x, mouse_y;

  while (running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT:
        running = false;
        break;
      case SDL_WINDOWEVENT: {
        if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
          board_resize_surface(board);
          board_clear(board);
          board_draw_strokes(board);
          board_render(board, NULL);
        }
      } break;
      case SDL_MOUSEBUTTONDOWN: {
        is_drawing = true;
        SDL_GetMouseState(&mouse_x, &mouse_y);
        vector_reset(board->current_stroke_points);
        vector_reset(board->current_stroke_paths);
        Point *current_pos = point_create(mouse_x, mouse_y);
        vector_append(board->current_stroke_points, current_pos);
        board_setup_draw(board);
        cairo_move_to(board->cr, mouse_x, mouse_y);
        cairo_arc(board->cr, mouse_x, mouse_y, 0, 0, TAU);
        cairo_path_t *point_path = cairo_copy_path(board->cr);
        vector_append(board->current_stroke_paths, point_path);
        cairo_stroke(board->cr);
        SDL_Rect bounds = {
            .x = mouse_x - LINE_WIDTH,
            .y = mouse_y - LINE_WIDTH,
            .w = 2 * LINE_WIDTH,
            .h = 2 * LINE_WIDTH,
        };
        board_render(board, &bounds);
      } break;
      case SDL_MOUSEBUTTONUP: {
        is_drawing = false;
        cairo_path_t *stroke = merge_paths(board->cr, board->current_stroke_paths);
        vector_append(board->strokes, stroke);
      } break;
      case SDL_MOUSEMOTION: {
        if (is_drawing == false) {
          break;
        }

        SDL_GetMouseState(&mouse_x, &mouse_y);
        Point *last_point = vector_top(board->current_stroke_points);
        if (last_point->x == mouse_x && last_point->y == mouse_y) {
          break;
        }

        Point *current_pos = point_create(mouse_x, mouse_y);
        vector_append(board->current_stroke_points, current_pos);

        // now that we know this data wont change,
        // alias it for readability.
        Vector *current_stroke = board->current_stroke_points;

        switch (current_stroke->length) {
        case 2: {
          Point *prev = vector_get(current_stroke, 0);
          cairo_move_to(board->cr, prev->x, prev->y);
        } break;
        case 3: {
          Point *origin = vector_get(current_stroke, 0);
          Point *dest = vector_get(current_stroke, 1);
          Point *next = vector_get(current_stroke, 2);

          Point *h1 = point_create(0, 0);
          Point *h2 = point_create(0, 0);

          create_handle_triple(origin, dest, next, h1, h2);
          cairo_move_to(board->cr, origin->x, origin->y);
          cairo_curve_to(board->cr, h1->x, h1->y, h2->x, h2->y, dest->x, dest->y);
        } break;
        default: {
          size_t last_index = current_stroke->length - 1;
          Point *prev = vector_get(current_stroke, last_index - 3);
          Point *origin = vector_get(current_stroke, last_index - 2);
          Point *dest = vector_get(current_stroke, last_index - 1);
          Point *next = vector_get(current_stroke, last_index);

          Point *h1 = point_create(0, 0);
          Point *h2 = point_create(0, 0);
          create_handle_quad(prev, origin, dest, next, h1, h2);
          cairo_move_to(board->cr, origin->x, origin->y);
          cairo_curve_to(board->cr, h1->x, h1->y, h2->x, h2->y, dest->x, dest->y);
        } break;
        }

        cairo_path_t *sub_path = cairo_copy_path(board->cr);
        vector_append(board->current_stroke_paths, sub_path);
        SDL_Rect bounds = get_path_bounding_area(board->cr);
        cairo_stroke(board->cr);
        board_render(board, &bounds);
      } break;
      case SDL_KEYDOWN: {
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        // ctrl+z -> undo last stroke
        if (keys[SDL_SCANCODE_LCTRL] && keys[SDL_SCANCODE_Z]) {
          if (board->strokes->length == 0) {
            break;
          }

          vector_pop(board->strokes);
          board_clear(board);
          board_draw_strokes(board);
          board_render(board, NULL);
        }
      } break;
      default:
        break;
      }
    }
    SDL_Delay(1000 / 60);
  }

  board_free(board);
  SDL_Quit();
}
