#include "data/list.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

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
  assert(list);
  struct node *new = malloc(sizeof(struct node));

  if (!new) {
    fprintf(stderr, "Failed to allocate memory: %d", errno);
    return;
  }

  if (!list->size) {
    list->first = new;
    new->next = NULL;
    new->prev = NULL;
    new->value = value;
    list->size++;
    return;
  }

  assert(node);
  assert(list->first);

  if (list->first == node) {
    list->first = new;
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

void gen_list_pop_front(list_T list) { gen_list_pop(list, list->first); };


struct node* gen_list_first(list_T list){
	return list->first;
};

struct node *gen_list_find(list_T list, void *value,
                           s32 (*cpm_fn)(void *a, void *b)) {
  struct node *n = list->first;
  while (n) {
    if (cpm_fn(value, n->value)) {
      return n;
    }
  }
  return NULL;
};
