#ifndef LIST_H
#define LIST_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct list *list_T;

#include "types.h"

struct node {
  struct node *next;
  struct node *prev;
  void *value;
};

// If we not return value a memory leak ocours becouse we cannot access it
// anymore. It would be nice to have two interfaces. One with a callback that
// automatically deletes items and the other that not frees value and returns it
// in case user need to use it more

extern list_T gen_list_new(void);

extern void gen_list_insert(list_T list, struct node *node, void *value);

extern void gen_list_push_front(list_T list, void *value);

extern void gen_list_pop(list_T list, struct node *node);

// nf -> no_free
// extern void list_pop_nf(list_T list, struct node *node);

extern void gen_list_pop_front(list_T list);

extern struct node* gen_list_first(list_T list);

/**
 * @brief Find the node with value 'value'
 *
 * @param list
 * @param value
 * @param cpm_fn callback function that must provide 1 on a == b and 0
 * otherwise
 * @return return a pointer to the node holding the value or NULL if value not
 * found in list
 */
extern struct node *gen_list_find(list_T list, void *value,
                           s32 (*cpm_fn)(void *a, void *b));

// nf -> no_free
// extern void list_pop_front_nf(list_T list);

// extern list_T list_new(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !LIST_H
