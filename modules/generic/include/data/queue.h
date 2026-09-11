#ifndef QUEUE_H
#define QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "types.h"

typedef struct queue *queue_T;


queue_T gen_queue_new(void);

void gen_queue_free(queue_T q, void (*free_value_fn)(void *value));

s32 gen_queue_push(queue_T q, void *value);

void *gen_queue_pop(queue_T q);

void *gen_queue_peek(queue_T q);

size_t gen_queue_size(queue_T q);

s32 gen_queue_empty(queue_T q);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !QUEUE_H
