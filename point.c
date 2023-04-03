#include "point.h"

#include <math.h>
#include <stdlib.h>

Point *point_create(double x, double y) {
  Point *p = malloc(sizeof(*p));
  p->x = x;
  p->y = y;
  return p;
}

void point_free(Point *p) {
  // TODO: fix clang format to deny one line functions
  free(p);
}

Point point_add(Point p1, Point p2) {
  double x = p1.x + p2.x;
  double y = p1.y + p2.y;
  Point p = {.x = x, .y = y};
  return p;
}

Point point_subtruct(Point p1, Point p2) {
  double x = p1.x - p2.x;
  double y = p1.y - p2.y;
  Point p = {.x = x, .y = y};
  return p;
}

Point point_multiply(Point p, double scalar) {
  double x = p.x * scalar;
  double y = p.y * scalar;
  Point result = {.x = x, .y = y};
  return result;
}

Point point_normalize(Point p) {
  double length = point_length(p);
  return point_multiply(p, 1.0 / length);
}

double point_length(Point p) {
  double point_length_squared = p.x * p.x + p.y * p.y;
  double point_length = sqrt(point_length_squared);
  return point_length;
}

void create_handle_triple(Point *p1, Point *p2, Point *p3, Point *h1, Point *h2) {
  Point parallel_direction_next = point_normalize(point_subtruct(*p3, *p1));
  Point delta = point_subtruct(*p2, *p1);
  Point ratio_delta = point_multiply(delta, RATIO);
  *h1 = point_add(*p1, ratio_delta);

  double delta_length = point_length(delta);
  Point handle_offset = point_multiply(parallel_direction_next, delta_length * RATIO);
  *h2 = point_subtruct(*p2, handle_offset);
}

void create_handle_quad(Point *p1, Point *p2, Point *p3, Point *p4, Point *h1, Point *h2) {
  Point parallel_direction_prev = point_normalize(point_subtruct(*p3, *p1));
  Point parallel_direction_next = point_normalize(point_subtruct(*p4, *p2));
  Point delta = point_subtruct(*p3, *p2);
  double delta_len = point_length(delta);

  *h1 = point_add(*p2, point_multiply(parallel_direction_prev, delta_len * RATIO));
  *h2 = point_subtruct(*p3, point_multiply(parallel_direction_next, delta_len * RATIO));
}
