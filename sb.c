#include "board.h"
#include "path.h"
#include "point.h"
#include "vector.h"
#include <SDL2/SDL_events.h>
#include <stdbool.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define BOUNDS_PADDING 3
#define FPS 60
#define FPS_DURATION (1000 / FPS)

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

SDL_Rect get_path_bounding_area(Board *board) {
  double x1, y1, x2, y2;
  cairo_stroke_extents(board->cr, &x1, &y1, &x2, &y2);
  int x = x1 + board->dx - BOUNDS_PADDING;
  int y = y1 + board->dy - BOUNDS_PADDING;
  int w = x2 - x1 + 2 * BOUNDS_PADDING;
  int h = y2 - y1 + 2 * BOUNDS_PADDING;
  SDL_Rect area = {.x = MAX(0, x), .y = MAX(0, y), .w = w, .h = h};
  return area;
}

void get_mousestate(Board *board, double *x, double *y) {
  int raw_x, raw_y;
  SDL_GetMouseState(&raw_x, &raw_y);
  *x = raw_x - board->dx;
  *y = raw_y - board->dy;
}

bool is_inside_rect(SDL_Rect *rect, double x, double y) {
  if (x <= rect->x || x >= rect->x + rect->w) {
    return false;
  }
  if (y <= rect->y || y >= rect->y + rect->h) {
    return false;
  }
  return true;
}

int main() {
  SDL_Init(SDL_INIT_VIDEO);
  Board *board = board_create(600, 480);
  bool running = true;
  double mouse_x, mouse_y;
  board_update_cursor(board);
  // used for evaluating fps
  Uint32 start, loop_duration;

  while (running) {
    SDL_Event event;
    start = SDL_GetTicks();
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT:
        running = false;
        break;
      case SDL_WINDOWEVENT: {
        if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
          board_resize_surface(board);
          board_refresh(board);
        }
      } break;
      case SDL_MOUSEBUTTONDOWN: {
        if (board->state != STATE_IDLE) {
          break;
        }

        if (event.button.button == SDL_BUTTON_LEFT) {
          get_mousestate(board, &mouse_x, &mouse_y);
          double raw_x = mouse_x + board->dx;
          double raw_y = mouse_y + board->dy;

#ifdef USE_TOOLBAR
          if (is_inside_rect(&board->toolbar_area, raw_x, raw_y)) {
            board_click_toolbar(board, raw_x);
            break;
          }
#endif

          board->state = STATE_DRAWING;
          board_reset_current_stroke(board);
          Point *current_pos = point_create(mouse_x, mouse_y);
          vector_append(board->current_stroke_points, current_pos);
          board_setup_draw(board);
          cairo_move_to(board->cr, mouse_x, mouse_y);
          cairo_arc(board->cr, mouse_x, mouse_y, 0, 0, M_PI * 2);
          cairo_path_t *point_path = cairo_copy_path(board->cr);
          vector_append(board->current_stroke_paths, point_path);
          cairo_stroke(board->cr);
          SDL_Rect bounds = {
              .x = mouse_x + board->dx - board->stroke_width,
              .y = mouse_y + board->dy - board->stroke_width,
              .w = 2 * board->stroke_width,
              .h = 2 * board->stroke_width,
          };
          board_render(board, &bounds);
          break;
        }

        if (event.button.button == SDL_BUTTON_RIGHT) {
          get_mousestate(board, &mouse_x, &mouse_y);
          // raw mouse position
          mouse_x += board->dx;
          mouse_y += board->dy;
          board->state = STATE_MOVING;
          break;
        }
      } break;
      case SDL_MOUSEBUTTONUP: {
        if (event.button.button == SDL_BUTTON_LEFT) {
          if (board->state != STATE_DRAWING) {
            break;
          }
          board->state = STATE_IDLE;
#ifdef USE_TOOLBAR
          if (board->toolbar->visible == false) {
            board->toolbar->visible = true;
            board_render(board, &board->toolbar_area);
          }
#endif
          cairo_path_t *stroke = merge_paths(board->cr, board->current_stroke_paths);
          cairo_new_path(board->cr);
          Path *colored_stroke = path_create(stroke, board->stroke_color, board->stroke_width);
          vector_append(board->strokes, colored_stroke);
          break;
        }

        if (event.button.button == SDL_BUTTON_RIGHT) {
          if (board->state != STATE_MOVING) {
            break;
          }
          board->state = STATE_IDLE;
          break;
        }
      } break;
      case SDL_MOUSEMOTION: {
        if (board->state == STATE_IDLE) {
          get_mousestate(board, &mouse_x, &mouse_y);
          double raw_x = mouse_x + board->dx;
          double raw_y = mouse_y + board->dy;
          SDL_ShowCursor(true);
#ifdef USE_TOOLBAR
          if (is_inside_rect(&board->toolbar_area, raw_x, raw_y)) {
            SDL_SetCursor(board->default_cursor);
          } else {
            SDL_SetCursor(board->cursor);
          }
#else
          SDL_SetCursor(board->cursor);
#endif

          break;
        }

        if (board->state == STATE_MOVING) {
          SDL_ShowCursor(false);
          // raws
          double initial_x = mouse_x;
          double initial_y = mouse_y;
          get_mousestate(board, &mouse_x, &mouse_y);
          mouse_x += board->dx;
          mouse_y += board->dy;

          double dx = mouse_x - initial_x;
          double dy = mouse_y - initial_y;
          board_translate(board, dx, dy);
          break;
        }

        // it is now guaranteed that board->state == STATE_DRAWING
        SDL_ShowCursor(true);
        get_mousestate(board, &mouse_x, &mouse_y);

        // determine visibility of the toolbar
        double raw_x = mouse_x + board->dx;
        double raw_y = mouse_y + board->dy;
#ifdef USE_TOOLBAR
        bool previous = board->toolbar->visible;
        if (is_inside_rect(&board->toolbar_area, raw_x, raw_y)) {
          board->toolbar->visible = false;
        } else {
          board->toolbar->visible = true;
        }
        // re-render toolbar if needed.
        if (previous != board->toolbar->visible) {
          board_render(board, &board->toolbar_area);
        }
#endif

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

          point_free(h1);
          point_free(h2);
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

          point_free(h1);
          point_free(h2);
        } break;
        }

        cairo_path_t *sub_path = cairo_copy_path(board->cr);
        vector_append(board->current_stroke_paths, sub_path);
        SDL_Rect bounds = get_path_bounding_area(board);
        cairo_stroke(board->cr);
        board_render(board, &bounds);
      } break;
      case SDL_KEYDOWN: {
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        // ctrl+z -> undo last stroke
        if (keys[SDL_SCANCODE_LCTRL] && keys[SDL_SCANCODE_Z]) {
          if (board->strokes->length != 0) {
            vector_pop(board->strokes);
            board_refresh(board);
          }
          break;
        }

        if (keys[SDL_SCANCODE_0]) {
          board_reset_translation(board);
          break;
        }

        if (keys[SDL_SCANCODE_MINUS]) {
#ifdef USE_TOOLBAR
          board->toolbar->selected_color = COLOR_PRIMARY;
#endif
          board->stroke_color = get_color(COLOR_PRIMARY);
          board_update_cursor(board);
          board_refresh(board);
          break;
        }

        if (keys[SDL_SCANCODE_EQUALS]) {
#ifdef USE_TOOLBAR
          board->toolbar->selected_color = COLOR_SECONDARY;
#endif
          board->stroke_color = get_color(COLOR_SECONDARY);
          board_update_cursor(board);
          board_refresh(board);
          break;
        }

        if (keys[SDL_SCANCODE_1]) {
#ifdef USE_TOOl
          board->toolbar->selected_width = STROKE_WIDTH_THIN;
#endif
          board->stroke_width = get_width(STROKE_WIDTH_THIN);
          board_update_cursor(board);
          board_refresh(board);
          break;
        }

        if (keys[SDL_SCANCODE_2]) {
#ifdef USE_TOOLBAR
          board->toolbar->selected_width = STROKE_WIDTH_MEDIUM;
#endif
          board->stroke_width = get_width(STROKE_WIDTH_MEDIUM);
          board_update_cursor(board);
          board_refresh(board);
          break;
        }

        if (keys[SDL_SCANCODE_3]) {
#ifdef USE_TOOLBAR
          board->toolbar->selected_width = STROKE_WIDTH_THICK;
#endif
          board->stroke_width = get_width(STROKE_WIDTH_THICK);
          board_update_cursor(board);
          board_refresh(board);
          break;
        }
      } break;
      default:
        break;
      }
    }

    loop_duration = SDL_GetTicks() - start;
    if (loop_duration <= FPS_DURATION) {
      SDL_Delay(FPS_DURATION - loop_duration);
    }
  }

  board_free(board);
  SDL_Quit();
}
