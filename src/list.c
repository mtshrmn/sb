#include "list.h"
#include "path.h"
#include "point.h"

static void listnode_free(ListNode *node) {
  void *data = node->data;
  switch (node->type) {
  case LIST_POINTS:
    point_free((Point *)data);
    break;
  case LIST_COLORED_PATHS:
    path_free((Path *)data);
    break;
  case LIST_PATHS:
    cairo_path_destroy((cairo_path_t *)data);
    break;
  }
  free(node);
}

List *list_create(ListType type) {
  List *list = malloc(sizeof(*list));
  if (list == NULL) {
    return NULL;
  }

  list->head = NULL;
  list->tail = NULL;
  list->length = 0;
  list->type = type;

  return list;
}

void list_reset(List *l) {
  ListNode *node = l->head;
  while (node != NULL) {
    ListNode *to_free = node;
    node = node->next;
    listnode_free(to_free);
  }
  l->head = NULL;
  l->tail = NULL;
  l->length = 0;
}

void list_free(List *l) {
  list_reset(l);
  free(l);
}

int list_append(List *l, void *item) {
  ListNode *node = malloc(sizeof(*node));
  if (node == NULL) {
    return 0;
  }

  node->data = item;
  node->type = l->type;
  node->next = NULL;
  node->prev = l->tail;

  if (l->length == 0) {
    l->head = node;
    l->tail = node;
    l->length = 1;
    return 1;
  }

  l->tail->next = node;
  l->tail = node;
  l->length++;
  return 1;
}

void list_pop(List *l) {
  if (l->tail == NULL) {
    return;
  }

  ListNode *node = l->tail;

  if (l->length == 1) {
    l->head = NULL;
    l->tail = NULL;
    l->length = 0;
    listnode_free(node);
    return;
  }

  l->tail = l->tail->prev;
  l->tail->next = NULL;
  l->length--;
  listnode_free(node);
  return;
}

void *list_top(List *l) {
  if (l->tail == NULL) {
    return NULL;
  }
  return l->tail->data;
}
