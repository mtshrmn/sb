#ifndef PDLL_H
#define PDLL_H

#include <stdbool.h>
#include <stddef.h>

typedef void (*pdll_free_node_data_func)(void *data);
typedef struct pdll_node {
  void *data;
  size_t version;
  struct pdll_node *next;
  struct pdll_node *prev;
  bool to_delete;
} pdll_node;

typedef struct {
  pdll_node *head;
  pdll_node *tail;
} pdll_version;

typedef struct {
  pdll_version *versions;
  pdll_free_node_data_func free_data;
  size_t latest_version;
  size_t capacity;
} pdll;

pdll *pdll_init(pdll_free_node_data_func free_data);
void pdll_free(pdll *list);
bool pdll_append(pdll *list, void *data);
void pdll_node_mark_for_deletion(pdll_node *node);
bool pdll_delete_marked_nodes(pdll *list);
bool pdll_undo(pdll *list);

#define pdll_iter(list, node)                                                                                          \
  for (pdll_node *node = (list)->versions[(list)->latest_version].head; node != NULL;                                  \
       node = (node == (list)->versions[(list)->latest_version].tail ? NULL : node->next))

#endif // PDLL_H
