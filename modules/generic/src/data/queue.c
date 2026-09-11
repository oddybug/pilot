#include "data/queue.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "data/list.h"
#include "log.h"
#include "types.h"

struct queue {
  list_T list;
};

queue_T gen_queue_new(void) {
  queue_T q = malloc(sizeof(struct queue));
  if (!q) {
    ERROR("Failed to allocate memory: %d", errno);
    return NULL;
  }

  q->list = gen_list_new();
  if (!q->list) {
    ERROR("Failed to allocate memory: %d", errno);
    free(q);
    return NULL;
  }

  return q;
};

void gen_queue_free(queue_T q, void (*free_value_fn)(void *value)) {
  assert(q);
  if (free_value_fn) {
    void *value;
    while ((value = gen_queue_pop(q)) != NULL)
      free_value_fn(value);
  }
  free(q->list);
  free(q);
};

s32 gen_queue_push(queue_T q, void *value) {
  assert(q);
  size_t before = gen_list_size(q->list);
  gen_list_push_back(q->list, value);
  if (gen_list_size(q->list) != before + 1) {
    ERROR("Failed to allocate memory: %d", errno);
    return 1;
  }
  return 0;
};

void *gen_queue_pop(queue_T q) {
  assert(q);
  if (gen_list_size(q->list) == 0)
    return NULL;
  return gen_list_pop_front(q->list);
};

void *gen_queue_peek(queue_T q) {
  assert(q);
  struct node *first = gen_list_first(q->list);
  if (!first)
    return NULL;
  return first->value;
};

size_t gen_queue_size(queue_T q) {
  assert(q);
  return gen_list_size(q->list);
};

s32 gen_queue_empty(queue_T q) {
  assert(q);
  return gen_list_size(q->list) == 0;
};
