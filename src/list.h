#ifndef SB_LIST_H
#define SB_LIST_H

#include <stdlib.h>
#define list_foreach(list, node) for (node = list->head; node != NULL; node = node->next)

typedef enum ListType {
  LIST_POINTS,
  LIST_PATHS,
  LIST_COLORED_PATHS,
} ListType;

typedef struct ListNode {
  void *data;
  ListType type;
  struct ListNode *prev;
  struct ListNode *next;
} ListNode;

typedef struct List {
  ListNode *head;
  ListNode *tail;
  size_t length;
  ListType type;
} List;

List *list_create(ListType type);
void list_reset(List *l);
void list_free(List *l);
int list_append(List *l, void *item);
void list_pop(List *l);
void *list_top(List *l);

#endif // SB_LIST_H
