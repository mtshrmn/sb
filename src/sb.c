#include "board.h"
#include "path.h"
#include "point.h"
#include <SDL2/SDL_events.h>
#include <stdbool.h>
#include <time.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define BOUNDS_PADDING 3
#define FPS 60
#define FPS_DURATION (1000 / FPS)

cairo_path_t *merge_paths(cairo_t *cr, List *paths) {
  // assume paths->type == VECTOR_PATHS
  cairo_new_path(cr);
  ListNode *node, *next_node;
  list_foreach(paths, node, next_node) {
    cairo_path_t *sub_path = node->data;
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

bool is_inside_rect(SDL_Rect *rect, double x, double y) {
  if (x <= rect->x || x >= rect->x + rect->w) {
    return false;
  }
  if (y <= rect->y || y >= rect->y + rect->h) {
    return false;
  }
  return true;
}

void on_window_event(Board *board, SDL_Event *sdl_event) {
  switch (sdl_event->window.event) {
  case SDL_WINDOWEVENT_RESIZED:
    board_resize_surface(board);
    board_refresh(board);
    break;
  case SDL_WINDOWEVENT_EXPOSED:
    board_refresh(board);
    break;
  }
}

void on_mouse_left_button_down(Board *board) {
  board_update_mouse_state(board);

  // draw the initial point where the user clicked.
  board->state = STATE_DRAWING;
  board_reset_current_stroke(board);
  Point *current_pos = point_create(board->mouse_x, board->mouse_y);
  list_append(board->current_stroke_points, current_pos);
  board_setup_draw(board);
  cairo_move_to(board->cr, board->mouse_x, board->mouse_y);
  cairo_arc(board->cr, board->mouse_x, board->mouse_y, 0, 0, M_PI * 2);
  cairo_path_t *point_path = cairo_copy_path(board->cr);
  list_append(board->current_stroke_paths, point_path);
  cairo_stroke(board->cr);
  SDL_Rect bounds = {
      .x = board->mouse_x_raw - board->stroke_width,
      .y = board->mouse_y_raw - board->stroke_width,
      .w = 2 * board->stroke_width,
      .h = 2 * board->stroke_width,
  };
  board_render(board, &bounds);
}
void on_mouse_right_button_down(Board *board) {
  board_update_mouse_state(board);
  board->state = STATE_MOVING;
  SDL_ShowCursor(false);
  return;
}

void on_mouse_button_down(Board *board, SDL_Event *event) {
  if (board->state != STATE_IDLE) {
    return;
  }

  switch (event->button.button) {
  case SDL_BUTTON_LEFT:
    on_mouse_left_button_down(board);
    break;
  case SDL_BUTTON_RIGHT:
    on_mouse_right_button_down(board);
    break;
  }
}

void on_mouse_left_button_up(Board *board) {
  if (board->state != STATE_DRAWING) {
    return;
  }

  cairo_path_t *stroke = merge_paths(board->cr, board->current_stroke_paths);
  cairo_new_path(board->cr);
  Path *colored_stroke = path_create(stroke, board->stroke_color, board->stroke_width);
  list_append(board->strokes, colored_stroke);
  board->state = STATE_IDLE;
}

void on_mouse_right_button_up(Board *board) {
  if (board->state == STATE_MOVING) {
    board->state = STATE_IDLE;
    SDL_ShowCursor(true);
  }
}

void on_mouse_button_up(Board *board, SDL_Event *event) {
  switch (event->button.button) {
  case SDL_BUTTON_LEFT:
    on_mouse_left_button_up(board);
    break;
  case SDL_BUTTON_RIGHT:
    on_mouse_right_button_up(board);
    break;
  }
}

void draw_smooth_stroke(Board *board, List *stroke) {
  switch (stroke->length) {
  case 2: {
    Point *prev = stroke->head->data;
    cairo_move_to(board->cr, prev->x, prev->y);
  } break;
  case 3: {
    Point *origin = stroke->head->data;
    Point *dest = stroke->head->next->data;
    Point *next = stroke->head->next->next->data;

    Point *h1 = point_create(0, 0);
    Point *h2 = point_create(0, 0);

    create_handle_triple(origin, dest, next, h1, h2);
    cairo_move_to(board->cr, origin->x, origin->y);
    cairo_curve_to(board->cr, h1->x, h1->y, h2->x, h2->y, dest->x, dest->y);

    point_free(h1);
    point_free(h2);
  } break;
  default: {
    Point *prev = stroke->tail->prev->prev->prev->data;
    Point *origin = stroke->tail->prev->prev->data;
    Point *dest = stroke->tail->prev->data;
    Point *next = stroke->tail->data;

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
  list_append(board->current_stroke_paths, sub_path);
  SDL_Rect bounds = get_path_bounding_area(board);
  cairo_stroke(board->cr);
  board_render(board, &bounds);
}

void on_mouse_motion(Board *board) {
  if (board->state == STATE_IDLE) {
    return;
  }

  if (board->state == STATE_MOVING) {
    double prev_raw_x = board->mouse_x_raw;
    double prev_raw_y = board->mouse_y_raw;
    board_update_mouse_state(board);
    double dx = board->mouse_x_raw - prev_raw_x;
    double dy = board->mouse_y_raw - prev_raw_y;
    board_translate(board, dx, dy);
    return;
  }

  // it is now guaranteed that board->state == STATE_DRAWING
  board_update_mouse_state(board);

  // don't draw the same point twice.
  Point *last_point = list_top(board->current_stroke_points);
  if (last_point->x == board->mouse_x && last_point->y == board->mouse_y) {
    return;
  }

  Point *current_pos = point_create(board->mouse_x, board->mouse_y);
  list_append(board->current_stroke_points, current_pos);

  // now that we know that board->current_stroke_points
  // won't mutate, alias it for readability.
  List *current_stroke = board->current_stroke_points;
  draw_smooth_stroke(board, current_stroke);
}

void on_key_down(Board *board) {
  const Uint8 *keys = SDL_GetKeyboardState(NULL);

  // ctrl+z -> undo last stroke
  if (keys[SDL_SCANCODE_LCTRL] && keys[SDL_SCANCODE_Z]) {
    if (board->strokes->length != 0) {
      list_pop(board->strokes);
      board_refresh(board);
    }
    return;
  }

  if (keys[SDL_SCANCODE_0]) {
    board_reset_translation(board);
    return;
  }

  if (keys[SDL_SCANCODE_1]) {
    if (board->stroke_color == BOARD_BG) {
      board_set_stroke_color(board, board->stroke_color_previous);
    }
    board_set_stroke_width(board, STROKE_WIDTH_THIN);
  }

  if (keys[SDL_SCANCODE_2]) {
    if (board->stroke_color == BOARD_BG) {
      board_set_stroke_color(board, board->stroke_color_previous);
    }
    board_set_stroke_width(board, STROKE_WIDTH_MEDIUM);
    return;
  }

  if (keys[SDL_SCANCODE_3]) {
    if (board->stroke_color == BOARD_BG) {
      board_set_stroke_color(board, board->stroke_color_previous);
    }
    board_set_stroke_width(board, STROKE_WIDTH_THICK);
    return;
  }

  if (keys[SDL_SCANCODE_MINUS]) {
    if (board->stroke_width == STROKE_WIDTH_THICKEST) {
      board_set_stroke_width(board, board->stroke_width_previous);
    }
    board_set_stroke_color(board, COLOR_PRIMARY);
  }

  if (keys[SDL_SCANCODE_EQUALS]) {
    if (board->stroke_width == STROKE_WIDTH_THICKEST) {
      board_set_stroke_width(board, board->stroke_width_previous);
    }
    board_set_stroke_color(board, COLOR_SECONDARY);
  }

  if (keys[SDL_SCANCODE_BACKSPACE]) {
    board->stroke_color_previous = board->stroke_color;
    board->stroke_width_previous = board->stroke_width;
    board_set_stroke_color(board, BOARD_BG);
    board_set_stroke_width(board, STROKE_WIDTH_THICKEST);
  }

  if (keys[SDL_SCANCODE_LCTRL] && keys[SDL_SCANCODE_S]) {
    time_t timer;
    time(&timer);
    struct tm *time_info = localtime(&timer);

    char filename[128];
    if (strftime(filename, sizeof(filename), SCREENSHOTS_PATH "sb_%Y_%m_%d-%H:%M:%S.png", time_info) == 0) {
      return;
    }

    board_save_image(board, filename);
  }
}

int main() {
  SDL_Init(SDL_INIT_VIDEO);
  Board *board = board_create(600, 480);
  bool running = true;
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
      case SDL_WINDOWEVENT:
        on_window_event(board, &event);
        break;
      case SDL_MOUSEBUTTONDOWN:
        on_mouse_button_down(board, &event);
        break;
      case SDL_MOUSEBUTTONUP:
        on_mouse_button_up(board, &event);
        break;
      case SDL_MOUSEMOTION:
        on_mouse_motion(board);
        break;
      case SDL_KEYDOWN:
        on_key_down(board);
        break;
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
