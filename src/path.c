#include "path.h"
#include <stdlib.h>

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
}
