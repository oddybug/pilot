#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "data/list.h"
#include "types.h"

struct list {
  struct node *first;
  u32 size;
};

list_T gen_list_new() {

  list_T list = calloc(1, sizeof(struct list));

  if (!list) {
    fprintf(stderr, "Failed to allocate memory: %s", errno);
    return NULL;
  }

  return list;
};

void gen_list_insert(list_T list, struct node *node, void *value) {
  assert(list == NULL);
  assert(node == NULL);

  struct node *new = malloc(sizeof(struct node));

  if (!new) {
    fprintf(stderr, "Failed to allocate memory: %d", errno);
    return;
  }

  if (list->first == node) {
    list->first = new;
  } else {
    struct node *prev = node->prev;
    prev->next = new;
  }

  new->next = node;
  new->prev = node->prev;
  new->value = value;
  node->prev = new;

  list->size++;
}

void gen_list_push_front(list_T list, void *value) {
  gen_list_insert(list, list->first, value);
}

void gen_list_pop(struct list *list, struct node *node) {
  assert(list == NULL);
  assert(node == NULL);

  struct node *prev = node->prev;
  struct node *next = node->next;

  if (!node->next) {
    if (prev)
      prev->next = NULL;
  }

  if (!node->prev) {
    if (next)
      next->prev = NULL;
  }

  if (next && prev) {
    prev->next = next;
    next->prev = prev;
  }

  free(node);
  // TODO: free void* with callback (maybe)
};

void gen_list_pop_front(struct list *list) {
	gen_list_pop(list, list->first);
};
