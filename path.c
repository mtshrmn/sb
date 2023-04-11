#include "path.h"
#include <stdlib.h>

Path *path_create(cairo_path_t *path, unsigned int color) {
  Path *p = malloc(sizeof(Path));
  if (p == NULL) {
    return NULL;
  }

  p->path = path;
  p->color = color;
  return p;
}

void path_free(Path *path) {
  cairo_path_destroy(path->path);
}

void extract_color(unsigned int color, double *r, double *g, double *b, double *a) {
  int red = (color & 0xFF000000) >> 24;
  int green = (color & 0x00FF0000) >> 16;
  int blue = (color & 0x0000FF00) >> 8;
  int alpha = (color & 0x000000FF);

  *r = red / 255.0;
  *g = green / 255.0;
  *b = blue / 255.0;
  *a = alpha / 255.0;
}
