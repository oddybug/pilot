#include <assert.h>
#include <errno.h>
#include <stdlib.h>

#include "data/hashmap.h"
#include "log.h"

struct item_T {
  struct item_T *next;
  void *key;
  void *value;
};

struct map_T {
  u32 size;
  struct item_T **bucket;

  u32 (*hash_fn)(const void *key);
  u32 (*cmp_fn)(const void *a, const void *b);
};

struct map_T *gen_map_create(u32 size, u32 (*hash_fn)(const void *key),
                             u32 (*cmp_fn)(const void *a, const void *b)) {

  struct item_T **bucket = calloc(1, size * sizeof(struct item_T *));

  assert(bucket);
  if (!bucket) {
    ERROR("Failed to calloc (errno: %d)", errno);
    return NULL;
  };

  struct map_T *res = malloc(sizeof(struct map_T));

  res->bucket = bucket;
  res->size = size;
  res->cmp_fn = cmp_fn;
  res->hash_fn = hash_fn;

  return res;
};

/**
 * @brief frees bucket row memory
 *
 * @param item 
 * @return 0 on succes 1 otherwise.
 */
static s32 gen_map_free_bucket_row_(struct item_T *item);

static s32 gen_map_free_bucket_row_(struct item_T *item) {
  assert(item);

  if (!item) {
    ERROR("Trying to delete item but pointer is NULL.");
    return 1;
  };

  while (item) {
    struct item_T *next = item->next;

    free(item->key);
    free(item->value);
    free(item);

    item = next;
  }

  return 0;
};

s32 gen_map_free(struct map_T *map) {
  assert(map);
  assert(map->bucket);

  if (!map) {
    ERROR("Trying to delete map but pointer is NULL.");
    return 1;
  }

  if (!map->bucket) {
    ERROR("Trying to delete map but bucket pointer is NULL.");
    free(map);
    return 1;
  }

  for (u32 i = 0; i < map->size; i++) {
    struct item_T *item = map->bucket[i];
    if (item) {
      gen_map_free_bucket_row_(item);
    }
  }

  free(map->bucket);
  free(map);

  return 0;
};


s32 gen_map_get_size(struct map_T *map){
	assert(map);
	if (!map){
	ERROR("can acces the size of a NULL map");
	return -1;
	}
	return map->size;
};

s32 gen_map_insert(struct map_T *map, void *key, void *value) {
  assert(key || map);
  if (!map || !key) {
    ERROR("map or key NULL.");
    return 1;
  }

  u32 hash = map->hash_fn(key);
  u32 index = hash % map->size;

  struct item_T *head = (struct item_T *)map->bucket[index];
  struct item_T *curr = head;

  while (curr != NULL) {
    if (map->cmp_fn(curr->key, key)) {
      curr->value = value;
      return 0;
    }
    curr = curr->next;
  }

  struct item_T *new_item = malloc(sizeof(struct item_T));
  if (!new_item) {
    ERROR("Couldn't allocate memory for the new item.");
    return 1;
  }

  new_item->key = key;
  new_item->value = value;
  new_item->next = head;
  map->bucket[index] = new_item;

  return 0;
}

s32 gen_map_remove(struct map_T *map, void *key) {
  assert(map);
  assert(key);

  if (!map || !key) {
    ERROR("map or key is NULL.");
    return 1;
  }

  u32 hash = map->hash_fn(key);
  u32 index = hash % map->size;

  struct item_T *curr = map->bucket[index];
  struct item_T *prev = NULL;

  while (curr != NULL) {
    if (map->cmp_fn(curr->key, key)) {
      if (prev == NULL) {
        map->bucket[index] = curr->next;
      } else {
        prev->next = curr->next;
      }

      free(curr->key);
      free(curr->value);
      free(curr);

      return 0;
    }

    prev = curr;
    curr = curr->next;
  }

  return 1;
}
