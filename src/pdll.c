#include "pdll.h"
#include <stdio.h>
#include <stdlib.h>

#define INIT_CAPACITY 256
#define GROWTH_RATE 2

static pdll_node *pdll_node_new(void *data, size_t version, pdll_node *prev, pdll_node *next) {
  pdll_node *node = malloc(sizeof(pdll_node));
  if (node == NULL) {
    return NULL;
  }

  node->data = data;
  node->version = version;
  node->prev = prev;
  node->next = next;
  node->to_delete = false;
  return node;
}

// free version without freeing the data
static void pdll_version_free(pdll_version *version) {
  for (pdll_node *n = version->head; n != NULL;) {
    pdll_node *next = n->next;
    free(n);
    n = next;
  }
  version->head = NULL;
  version->tail = NULL;
}

static bool pdll_ensure_capacity(pdll *list) {
  if (list == NULL) {
    return false;
  }

  if (list->latest_version + 1 < list->capacity) {
    return true;
  }

  size_t new_capacity = list->capacity * GROWTH_RATE;
  pdll_version *tmp = realloc(list->versions, sizeof(*tmp) * new_capacity);
  if (tmp == NULL) {
    return false;
  }

  list->versions = tmp;
  list->capacity = new_capacity;
  return true;
}

void pdll_node_mark_for_deletion(pdll_node *node) {
  node->to_delete = true;
}

pdll *pdll_init(pdll_free_node_data_func free_data) {
  pdll *list = malloc(sizeof(pdll));
  if (list == NULL) {
    return NULL;
  }

  pdll_version *versions = malloc(sizeof(*versions) * INIT_CAPACITY);
  if (versions == NULL) {
    free(list);
    return NULL;
  }

  versions[0].head = NULL;
  versions[0].tail = NULL;

  list->versions = versions;
  list->free_data = free_data;
  list->latest_version = 0;
  list->capacity = INIT_CAPACITY;
  return list;
}

bool pdll_append(pdll *list, void *data) {
  if (list == NULL) {
    return false;
  }

  if (pdll_ensure_capacity(list) == false) {
    return false;
  }

  size_t current_version_idx = list->latest_version;
  pdll_version *current_version = &list->versions[current_version_idx];

  size_t new_version_idx = current_version_idx + 1;
  pdll_version *new_version = &list->versions[new_version_idx];

  pdll_node *new_node = pdll_node_new(data, new_version_idx, current_version->tail, NULL);
  if (new_node == NULL) {
    return false;
  }

  new_version->head = current_version->head ? current_version->head : new_node;
  new_version->tail = new_node;
  if (current_version->tail) {
    current_version->tail->next = new_node;
  }

  list->latest_version = new_version_idx;
  return true;
}

bool pdll_delete_marked_nodes(pdll *list) {
  if (list == NULL) {
    return false;
  }

  if (pdll_ensure_capacity(list) == false) {
    return false;
  }

  size_t current_version_idx = list->latest_version;
  pdll_version *current_version = &list->versions[current_version_idx];

  size_t new_version_idx = current_version_idx + 1;
  pdll_version *new_version = &list->versions[new_version_idx];

  new_version->head = NULL;
  new_version->tail = NULL;
  pdll_node *prev_copy = NULL;
  pdll_node *node = current_version->head;

  while (node != NULL) {
    if (node->to_delete) {
      // reset marking of current version
      node->to_delete = false;
      goto next_iteration;
    }

    pdll_node *copy = pdll_node_new(node->data, new_version_idx, prev_copy, NULL);
    if (copy == NULL) {
      pdll_version_free(new_version);
      return false;
    }

    copy->version = node->version;

    if (prev_copy != NULL) {
      prev_copy->next = copy;
    } else {
      new_version->head = copy;
    }
    new_version->tail = copy;
    prev_copy = copy;

  next_iteration:
    if (node == current_version->tail) {
      break;
    }

    node = node->next;
  }

  list->latest_version = new_version_idx;
  return true;
}

bool pdll_undo(pdll *list) {
  if (list == NULL) {
    return false;
  }

  if (list->latest_version == 0) {
    return false;
  }

  pdll_version *current_version = &list->versions[list->latest_version];
  size_t previous_version_idx = list->latest_version - 1;
  pdll_version *previous_version = &list->versions[previous_version_idx];

  if (current_version->head == previous_version->head) {
    // this version was created from append on a non-empty version
    // meaning that this version has at least 2 nodes.
    // delete last node and free its data
    pdll_node *tail = current_version->tail;
    list->free_data(tail->data);
    current_version->tail = tail->prev;
    current_version->tail->next = NULL;
    free(tail);
  } else {
    // this version was created from delete or an append on an empty version
    // delete all the nodes in this version,
    // but remove only the data that corresponds to the version.
    pdll_node *node = current_version->tail;
    while (node != NULL) {
      pdll_node *prev = node->prev;

      if (node->version == list->latest_version) {
        list->free_data(node->data);
      }

      free(node);
      node = prev;
    }
    current_version->head = NULL;
    current_version->tail = NULL;
  }

  list->latest_version--;
  return true;
}

void pdll_free(pdll *list) {
  if (list == NULL) {
    return;
  }

  while (list->latest_version > 0) {
    pdll_undo(list);
  }

  free(list->versions);
  free(list);
}
