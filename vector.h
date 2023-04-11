#ifndef SB_VECTOR_H
#define SB_VECTOR_H

#include <stdlib.h>

#define GROWTH_RATE 2

typedef enum VectorType {
  VECTOR_POINTS,
  VECTOR_PATHS,
  VECTOR_COLORED_PATHS,
} VectorType;

typedef struct Vector {
  void **data;
  size_t size;
  size_t length;
  VectorType type;
} Vector;

Vector *vector_create(VectorType type);
void vector_reset(Vector *vec);
void vector_free(Vector *vec);
void vector_append(Vector *vec, void *item);
void vector_pop(Vector *vec);
void *vector_get(Vector *vec, size_t index);
void *vector_top(Vector *vec);

#endif // SB_VECTOR_H
