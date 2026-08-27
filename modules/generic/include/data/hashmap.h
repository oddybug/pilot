#ifndef HASHMAP_H
#define HASHMAP_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "types.h"

struct item_T {
  struct item_T *next;
  void *key;
  void *value;
};

struct map_it_T {
  struct item_T *current;
  u32 index;
};

typedef struct map *map_T;

/**
 * @brief initialize a hashmap of size 'size'
 *
 * @param size the size of the map keys
 * @param hash_fn hashing funtion (use gen_map_get_size for getting map mod as
 * it might grow or shrink after initialitzation).
 * @param cmp_fn comparing function. Return value must be 1 on true 0 on false
 * @param free this function calls when deleting an item of the map if its not
 * NULL. Make sure to deallocate all memory inside the 'value' of the map item
 * if you used malloc or similars.
 * @return a pointer to the hashmap or NULL if failed to allocate memory
 */
extern map_T gen_map_create(u32 size, u32 (*hash_fn)(const void *key),
                            u32 (*compare_fn)(const void *a, const void *b),
                            void (*free)(struct item_T *item));

/**
 * @brief deletes the map
 *
 * @param map
 * @return returns 0 on succes 1 otherwise
 */
extern s32 gen_map_free(map_T map);

/**
 * @brief get the size of the map
 *
 * @param map
 * @return the size of the map. If map is NULL return value is -1;
 */
extern s32 gen_map_get_size(map_T map);

/**
 * @brief Inserts a new value, key pair into the map. If the key already exists
 * it updates the value with the provided one.
 *
 * @param map
 * @param key
 * @param value
 * @return 0 on succes 1 otherwise.
 */
extern s32 gen_map_insert(map_T map, const void *key, void *value);

/**
 * @brief deletes item with key 'key' from the map.
 *
 * @param map
 * @param key
 * @return returns 0 on succes 1 otherwise.
 */
extern s32 gen_map_remove(map_T map, const void *key);

/**
 * @brief Try find if a value with assosiated key exists
 *
 * @param map
 * @param key
 * @return NULL if key not in map or value otherwise.
 */
extern void *gen_map_find(map_T map, const void *key);

/**
 * @brief check if 'it' is past the last item of the map
 *
 * @param map
 * @param it
 * @return 1 if true 0 otherwise
 */
extern s32 gen_map_it_is_end(map_T map, struct map_it_T *it);

/**
 * @brief set the iterator to the fist item of the map
 *
 * @param map
 * @param it
 */
extern void gen_map_it_set_begin(map_T map, struct map_it_T *it);

/**
 * @brief Set it.next to the next item of the map
 *
 * @param map
 * @param it
 *
 * @result 'it->current' and 'it->index' update to the next item_T. If no
 * further elements exist, `it->current` will be set to NULL.
 */
extern void gen_map_it_get_next(map_T map, struct map_it_T *it);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !HASHMAP_H
