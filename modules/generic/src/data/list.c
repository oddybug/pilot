#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "data/list.h"
#include "types.h"

struct list_T {
  struct node_T *first;
  u32 size;
};

extern struct list_T *list_new() {

  struct list_T *list = calloc(1, sizeof(struct list_T));

  if (!list) {
    fprintf(stderr, "Failed to allocate memory: %s", errno);
    return NULL;
  }

  return list;
};

extern void list_insert(struct list_T *list, struct node_T *node, void *value) {
  // assert(list == NULL); TODO: ASSERTION
  // assert(node == NULL); TODO: ASSERTION

  struct node_T *new = malloc(sizeof(struct node_T));

  if (!new) {
    fprintf(stderr, "Failed to allocate memory: %d", errno);
    return;
    // TODO: EXCEPTION
  }

  new->next = node->next;
  new->value = value;
  node->next = new;

  list->size++;
}

extern void list_push_front(struct list_T *list, void *value) {
  // assert(list == NULL); TODO: ASSERTION

  struct node_T *new = malloc(sizeof(struct node_T));

  if (!new) {
    fprintf(stderr, "Failed to allocate memory: %d", errno);
    return;
    // TODO: EXCEPTION
  }

  new->value = value;

  if (list->first == NULL) {
    list->first = new;
  } else {
    new->next = list->first;
    list->first = new;
  }

  list->size++;
}

extern void list_pop(struct list_T *list, struct node_T *node) {
  // assert(list == NULL)
  // assert(node == NULL)

  if (node->next == NULL) {
    free(list->first);
    return;
  }

  struct node_T *next = node->next;
  *node = *next;

  free(next);
};

extern void list_pop_front(struct list_T *list, void *value) {
  // assert(list == NULL)
  if (list->first == NULL) {
    fprintf(stderr, "List already empty.");
    return;
  }

  list->first = list->first->next;
  free(list->first);
  list->size--;
};
