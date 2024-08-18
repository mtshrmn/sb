#include "list.h"

static void listnode_free(ListNode *node) {
  if (node == NULL) {
    return;
  }

  if (node->free_data != NULL && node->data != NULL) {
    node->free_data(node->data);
  }

  free(node);
}

List *list_create(list_free_function free_data_function) {
  List *list = malloc(sizeof(*list));
  if (list == NULL) {
    return NULL;
  }

  list->head = NULL;
  list->tail = NULL;
  list->length = 0;
  list->free_data = free_data_function;

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
  node->free_data = l->free_data;
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

void list_remove(List *l, ListNode *node) {
  if (l == NULL || node == NULL || l->length == 0) {
    return;
  }

  // Update head or tail if necessary
  if (l->head == node) {
    l->head = node->next;
  }
  if (l->tail == node) {
    l->tail = node->prev;
  }

  // Update the neighboring nodes
  if (node->prev != NULL) {
    node->prev->next = node->next;
  }
  if (node->next != NULL) {
    node->next->prev = node->prev;
  }

  listnode_free(node);
  l->length--;

  // If the list is now empty, reset head and tail
  if (l->length == 0) {
    l->head = NULL;
    l->tail = NULL;
  }
}

int list_contains(List *l, void *item) {
  ListNode *node, *next_node;
  list_foreach(l, node, next_node) {
    if (node->data == item) {
      return 1;
    }
  }

  return 0;
}

size_t list_getlen(List *l) {
  size_t len = 0;
  ListNode *stroke, *stroke_next;
  list_foreach(l, stroke, stroke_next) {
    len++;
  }

  return len;
}
