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
  if (length == 0) {
    // point is (0,0), don't normalize it.
    return p;
  }
  return point_multiply(p, 1.0 / length);
}

double point_length(Point p) {
  double point_length_squared = p.x * p.x + p.y * p.y;
  double point_length = sqrt(point_length_squared);
  return point_length;
}

double point_dist2(Point a, Point b) {
  double dx = b.x - a.x;
  double dy = b.y - a.y;
  return dx * dx + dy * dy;
}

double point_dist2_to_segment(Point p, Point a, Point b) {
  double len2 = point_dist2(a, b);
  if (len2 == 0) {
    return point_dist2(p, a);
  }

  double dx = b.x - a.x;
  double dy = b.y - a.y;
  // projection parameter - dot product of (p - a) and (b - a)
  double t = ((p.x - a.x) * dx + (p.y - a.y) * dy) / len2;
  // clamp it to be on the segment
  t = t < 0 ? 0 : (t > 1 ? 1 : t);
  Point closest = {
      .x = a.x + t * dx,
      .y = a.y + t * dy,
  };
  return point_dist2(closest, p);
}

// B(t) = (1 - t)^3 * p0 + 3(1-t)^2t * p1 + 3(1-t)t^2 * p2 + t^3 * p3
Point point_bezier_at(double t, Point p0, Point p1, Point p2, Point p3) {
  double u = 1.0 - t;
  double u2 = u * u;
  double u3 = u2 * u;
  double t2 = t * t;
  double t3 = t2 * t;
  Point result = {
      .x = u3 * p0.x + 3 * u2 * t * p1.x + 3 * u * t2 * p2.x + t3 * p3.x,
      .y = u3 * p0.y + 3 * u2 * t * p1.y + 3 * u * t2 * p2.y + t3 * p3.y,
  };
  return result;
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
