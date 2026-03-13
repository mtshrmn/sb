#ifndef SB_PATH_H
#define SB_PATH_H

#include "list.h"
#include "point.h"
#include <cairo/cairo.h>
#include <stdbool.h>

typedef struct {
  cairo_path_t *path;
  unsigned int color; // store color as 0xRRGGBBAA
  double width;
} Path;

Path *path_create(cairo_path_t *path, unsigned int color, double width);
void path_free(Path *path);

#endif
