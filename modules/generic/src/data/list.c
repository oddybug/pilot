#include "data/list.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "log.h"
#include "types.h"

struct list {
  struct node *first;
  struct node *last;
  size_t size;
};

list_T gen_list_new(void) {

  list_T list = calloc(1, sizeof(struct list));

  if (!list) {
    ERROR("Failed to allocate memory: %d", errno);
    return NULL;
  }

  return list;
};

void gen_list_insert(list_T list, struct node *node, void *value) {
  assert(list);

  if (!list->size) {
    gen_list_push_back(list, value);
    return;
  }

  assert(node);

  struct node *new = malloc(sizeof(struct node));
  if (!new) {
    ERROR("Failed to allocate memory: %d\n", errno);
    return;
  }

  new->value = value;
  new->next = node;
  new->prev = node->prev;

  if (node->prev) {
    node->prev->next = new;
  } else {
    list->first = new;
  }

  node->prev = new;
  list->size++;
}

void gen_list_push_front(list_T list, void *value) {
  gen_list_insert(list, list->first, value);
}

void gen_list_push_back(list_T list, void *value) {
  assert(list);

  struct node *new = malloc(sizeof(struct node));
  if (!new) {
    ERROR("Failed to allocate memory: %d\n", errno);
    return;
  }

  new->value = value;
  new->next = NULL;
  new->prev = list->last;

  if (!list->size) {
    list->first = new;
    list->last = new;
  } else {
    list->last->next = new;
    list->last = new;
  }

  list->size++;
}

void gen_list_remove(struct list *list, struct node *node) {
  assert(list);
  assert(node);

  if (node->prev) {
    node->prev->next = node->next;
  } else {
    list->first = node->next;
  }

  if (node->next) {
    node->next->prev = node->prev;
  } else {
    list->last = node->prev;
  }

  free(node);
  list->size--;
  // TODO: free void* with callback (maybe)
};

void *gen_list_pop(list_T list) {
  assert(list);
  assert(list->last);
  void *cpy = list->last->value;
  gen_list_remove(list, list->last);
  return cpy;
};

void *gen_list_pop_front(list_T list) {
  assert(list);
  assert(list->first);
  void *cpy = list->first->value;
  gen_list_remove(list, list->first);
  return cpy;
};

struct node *gen_list_first(list_T list) {
  assert(list);
  return list->first;
};

size_t gen_list_size(list_T list) {
  assert(list);
  return list->size;
};

struct node *gen_list_find(list_T list, void *value,
                           s32 (*cpm_fn)(void *a, void *b)) {
  struct node *n = list->first;
  while (n) {
    if (cpm_fn(value, n->value)) {
      return n;
    }
    n = n->next;
  }
  return NULL;
};
