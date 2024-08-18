#ifndef SB_LIST_H
#define SB_LIST_H

#include <stdlib.h>

#define list_foreach(list, node, next_node)                                                                            \
  for ((node) = (list)->head, (next_node) = ((node) ? (node)->next : NULL); (node) != NULL;                            \
       (node) = (next_node), (next_node) = ((node) ? (node)->next : NULL))

typedef void (*list_free_function)(void *);

typedef struct ListNode {
  void *data;
  list_free_function free_data;
  struct ListNode *prev;
  struct ListNode *next;
} ListNode;

typedef struct List {
  ListNode *head;
  ListNode *tail;
  size_t length;
  list_free_function free_data;
} List;

List *list_create(list_free_function free_data_function);
void list_reset(List *l);
void list_free(List *l);
int list_append(List *l, void *item);
void list_pop(List *l);
void *list_top(List *l);
void list_remove(List *l, ListNode *node);
int list_contains(List *l, void *item);
size_t list_getlen(List *l);

#endif // SB_LIST_H
