#ifndef LIST_H
#define LIST_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

struct list_T;

struct node_T {
  struct node_T *next;
  void *value;
};

extern struct list_T *list_new(void);

extern void list_insert(struct list_T *list, struct node_T *node, void *value);

extern void list_push_front(struct list_T *list, void *value);

extern void list_pop(struct list_T *list, struct node_T *node);

extern void list_pop_front(struct list_T *list, void *value);

extern struct list_T *list_new(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !LIST_H
