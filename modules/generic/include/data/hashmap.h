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

struct map_T;

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
extern struct map_T *gen_map_create(u32 size, u32 (*hash_fn)(const void *key),
                                    u32 (*compare_fn)(const void *a,
                                                      const void *b),
                                    void (*free)(struct item_T *item));

/**
 * @brief deletes the map
 *
 * @param map
 * @return returns 0 on succes 1 otherwise
 */
extern s32 gen_map_free(struct map_T *map);

/**
 * @brief get the size of the map
 *
 * @param map
 * @return the size of the map. If map is NULL return value is -1;
 */
extern s32 gen_map_get_size(struct map_T *map);

/**
 * @brief Inserts a new value, key pair into the map. If the key already exists
 * it updates the value with the provided one.
 *
 * @param map
 * @param key
 * @param value
 * @return 0 on succes 1 otherwise.
 */
extern s32 gen_map_insert(struct map_T *map, void *key, void *value);

/**
 * @brief deletes item with key 'key' from the map.
 *
 * @param map
 * @param key
 * @return returns 0 on succes 1 otherwise.
 */
extern s32 gen_map_remove(struct map_T *map, void *key);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !HASHMAP_H
