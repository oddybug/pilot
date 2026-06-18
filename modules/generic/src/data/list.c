#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "data/list.h"
#include "types.h"

#define T list_T
#define O node_T

struct T {
  O first;
  u32 size;
};

extern T list_new() {

  T list = calloc(1, sizeof(struct T));

  if (!list) {
    fprintf(stderr, "Failed to allocate memory: %s", errno);
    return NULL;
  }

  return list;
};

extern void list_insert(T list, O node, void *value) {
  // assert(list == NULL); TODO: ASSERTION
  // assert(node == NULL); TODO: ASSERTION

  O new = malloc(sizeof(struct O));

  if (!new) {
    fprintf(stderr, "Failed to allocate memory: %s", errno);
    return;
    // TODO: EXCEPTION
  }

  new->next = node->next;
  new->value = value;
  node->next = new;

  list->size++;
}

extern void list_push_front(T list, void *value) {
  // assert(list == NULL); TODO: ASSERTION

  O new = malloc(sizeof(struct O));

  if (!new) {
    fprintf(stderr, "Failed to allocate memory: %s", errno);
    return;
    // TODO: EXCEPTION
  }

  if (list->first == NULL) {
    list->first = new;
  } else {
    new->next = list->first->next;
    list->first = new;
  }

  list->size++;
}

extern void list_pop(T list, O node) {
  // assert(list == NULL)
  // assert(node == NULL)

  if (node->next == NULL) {
    free(list->first);
    return;
  }

  O next = node->next;
  *node = *next;

  free(next);
};

extern void list_pop_front(T list, void *value) {
  // assert(list == NULL)
  if (list->first == NULL) {
    fprintf(stderr, "List already empty.");
    return;
  }

  list->first = list->first->next;
  free(list->first);
  list->size--;
};

#undef T
#undef O
