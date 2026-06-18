#ifndef LIST_H

#ifdef __cplusplus
extern "C" {

#endif // __cplusplus

#define T list_T
#define O node_T

typedef struct T *T;
typedef struct O *O;

struct O {
  O next;
  void *value;
};

extern T list_new(void);

extern void list_insert(T list, O node, void *value);

extern void list_push_front(T list, void *value);

extern void list_pop(T list, O node);

extern void list_pop_front(T list, void *value);

extern T list_new(void);

#undef T
#undef O
#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !LIST_H
