#include "path.h"
#include <stdlib.h>
#include <string.h>

#define PATH_FLATTEN_CURVE_SAMPLES 16

Path *path_create(cairo_path_t *path, unsigned int color, double width) {
  Path *p = malloc(sizeof(Path));
  if (p == NULL) {
    return NULL;
  }

  p->path = path;
  p->color = color;
  p->width = width;
  return p;
}

void path_free(Path *path) {
  cairo_path_destroy(path->path);
  free(path);
}

Point *path_flatten(const cairo_path_t *cairo_path, int *len) {
  int max_size = (cairo_path->num_data + 1) * (PATH_FLATTEN_CURVE_SAMPLES + 1);
  Point *pts = malloc(max_size * sizeof(Point));
  if (pts == NULL) {
    *len = 0;
    return NULL;
  }

  int n = 0;
  Point prev = {.x = 0, .y = 0};

  for (int i = 0; i < cairo_path->num_data;) {
    const cairo_path_data_t *d = &cairo_path->data[i];
    switch (d[0].header.type) {
    case CAIRO_PATH_MOVE_TO: {
      Point p = {.x = d[1].point.x, .y = d[1].point.y};
      prev = p;
      pts[n++] = prev;
      break;
    }

    case CAIRO_PATH_LINE_TO: {
      Point p = {.x = d[1].point.x, .y = d[1].point.y};
      prev = p;
      pts[n++] = prev;
      break;
    }

    case CAIRO_PATH_CURVE_TO: {
      Point p1 = {.x = d[1].point.x, .y = d[1].point.y};
      Point p2 = {.x = d[2].point.x, .y = d[2].point.y};
      Point p3 = {.x = d[3].point.x, .y = d[3].point.y};
      for (int s = 1; s <= PATH_FLATTEN_CURVE_SAMPLES; s++) {
        pts[n++] = point_bezier_at((double)s / PATH_FLATTEN_CURVE_SAMPLES, prev, p1, p2, p3);
      }
      prev = p3;
      break;
    }

    case CAIRO_PATH_CLOSE_PATH:
      break;
    }

    i += d[0].header.length;
  }

  *len = n;
  return pts;
}

bool path_polylines_intersect(const Point *pts_a, int count_a, const Point *pts_b, int count_b, double threshold) {
  double t2 = threshold * threshold;

  for (int i = 0; i + 1 < count_a; i++) {
    for (int j = 0; j < count_b; j++) {
      if (point_dist2_to_segment(pts_b[j], pts_a[i], pts_a[i + 1]) <= t2)
        return true;
    }
  }

  // single-point eraser (click without drag)
  if (count_a == 1) {
    for (int j = 0; j + 1 < count_b; j++) {
      if (point_dist2_to_segment(pts_a[0], pts_b[j], pts_b[j + 1]) <= t2)
        return true;
    }
  }

  return false;
}
