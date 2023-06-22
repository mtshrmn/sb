#include "vector.h"
#include "path.h"
#include "point.h"
#include <cairo/cairo.h>
#include <stdio.h>

Vector *vector_create(VectorType type) {
  Vector *vec = malloc(sizeof(*vec));
  if (vec == NULL) {
    return NULL;
  }

  void **data = malloc(sizeof(void *));
  if (data == NULL) {
    return NULL;
  }

  vec->data = data;
  vec->size = 1;
  vec->length = 0;
  vec->type = type;
  return vec;
}

void vector_reset(Vector *vec) {
  if (vec->type == VECTOR_POINTS) {
    for (size_t i = 0; i < vec->length; ++i) {
      Point *point = vector_get(vec, i);
      point_free(point);
    }
  } else if (vec->type == VECTOR_COLORED_PATHS) {
    for (size_t i = 0; i < vec->length; ++i) {
      Path *path = vector_get(vec, i);
      path_free(path);
    }
  } else { // data is of type cairo_path_t
    for (size_t i = 0; i < vec->length; ++i) {
      cairo_path_t *path = vector_get(vec, i);
      cairo_path_destroy(path);
    }
  }
  vec->length = 0;
}

void vector_free(Vector *vec) {
  vector_reset(vec);
  free(vec->data);
  free(vec);
  vec = NULL;
}

void vector_append(Vector *vec, void *item) {
  if (vec->size == vec->length) {
    size_t new_size = vec->size * GROWTH_RATE;
    void **new_data = realloc(vec->data, new_size * sizeof(void *));
    if (new_data == NULL) {
      return;
    }
    vec->size = new_size;
    vec->data = new_data;
  }

  vec->data[vec->length] = item;
  vec->length++;
}

void vector_pop(Vector *vec) {
  void *last_item = vector_top(vec);
  if (vec->type == VECTOR_POINTS) {
    free(last_item);
  } else if (vec->type == VECTOR_COLORED_PATHS) {
    path_free(last_item);
  } else { // data is of type cairo_path_t
    cairo_path_destroy(last_item);
  }
  vec->length--;
}

void *vector_get(Vector *vec, size_t index) {
  if (index >= vec->length) {
    return NULL;
  }

  return vec->data[index];
}

void *vector_top(Vector *vec) {
  size_t last_index = vec->length - 1;
  return vec->data[last_index];
}
