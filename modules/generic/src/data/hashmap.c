#include <assert.h>
#include <errno.h>
#include <stdlib.h>

#include "data/hashmap.h"
#include "log.h"

struct map_T {
  u32 b_num;
  struct item_T **bucket;

  u32 (*hash_fn)(const void *key);
  u32 (*cmp_fn)(const void *a, const void *b);
  void (*free_item)(struct item_T *item);
};

static void gen_map_free_item_(struct map_T *map, struct item_T *item);

static void gen_map_free_item_(struct map_T *map, struct item_T *item) {
  if (map->free_item)
    map->free_item(item);

  free(item->key);
  free(item->value);
  free(item);
};

struct map_T *gen_map_create(u32 size, u32 (*hash_fn)(const void *key),
                             u32 (*cmp_fn)(const void *a, const void *b),
                             void (*free_item)(struct item_T *item)) {

  struct item_T **bucket = calloc(1, size * sizeof(struct item_T *));

  assert(bucket);
  if (!bucket) {
    ERROR("Failed to calloc (errno: %d)", errno);
    return NULL;
  };

  struct map_T *res = malloc(sizeof(struct map_T));

  res->bucket = bucket;
  res->b_num = size;
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
static s32 gen_map_free_bucket_row_(struct map_T *map, struct item_T *item);

static s32 gen_map_free_bucket_row_(struct map_T *map, struct item_T *item) {
  assert(item);

  if (!item) {
    ERROR("Trying to delete item but pointer is NULL.");
    return 1;
  };

  while (item) {
    struct item_T *next = item->next;

    gen_map_free_item_(map, item);
    item = next;
  }

  return 0;
};

s32 gen_map_free(struct map_T *map) {
  assert(map && map->bucket);

  if (!map) {
    ERROR("Trying to delete map but pointer is NULL.");
    return 1;
  }

  if (!map->bucket) {
    ERROR("Trying to delete map but bucket pointer is NULL.");
    free(map);
    return 1;
  }

  for (u32 i = 0; i < map->b_num; i++) {
    struct item_T *item = map->bucket[i];
    if (item) {
      gen_map_free_bucket_row_(map, item);
    }
  }

  free(map->bucket);
  free(map);

  return 0;
};

s32 gen_map_get_size(struct map_T *map) {
  assert(map);
  if (!map) {
    ERROR("can acces the size of a NULL map");
    return -1;
  }
  return map->b_num;
};

s32 gen_map_insert(struct map_T *map, const void *key, void *value) {
  assert(key && map);
  if (!map || !key) {
    ERROR("map or key NULL.");
    return 1;
  }

  u32 hash = map->hash_fn(key);
  u32 index = hash % map->b_num;

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

s32 gen_map_remove(struct map_T *map, const void *key) {
  assert(map && key);

  if (!map || !key) {
    ERROR("map or key is NULL.");
    return 1;
  }

  u32 hash = map->hash_fn(key);
  u32 index = hash % map->b_num;

  struct item_T *curr = map->bucket[index];
  struct item_T *prev = NULL;

  while (curr != NULL) {
    if (map->cmp_fn(curr->key, key)) {
      if (prev == NULL) {
        map->bucket[index] = curr->next;
      } else {
        prev->next = curr->next;
      }

      gen_map_free_item_(map, curr);
      return 0;
    }

    prev = curr;
    curr = curr->next;
  }

  return 1;
}

extern void *gen_map_find(struct map_T *map, const void *key) {
  assert(map && key);

  u32 hash = map->hash_fn(key);
  u32 index = hash % map->b_num;

  struct item_T *curr = map->bucket[index];
  struct item_T *prev = NULL;

  while (curr != NULL) {
    if (map->cmp_fn(curr->key, key)) {
      return curr->value;
    }

    prev = curr;
    curr = curr->next;
  }

  return NULL;
};

/**
 * @brief helper function to get next item_T of the iterator it. it->current
 * must be not NULL.
 *
 * @param map
 * @param it
 * @result 'it->current' and 'it->index' update to the next item_T. If no
 * further elements exist, `it->current` will be set to NULL.
 */
static void gen_map_it_get_next_(struct map_T *map, struct map_it_T *it);

static void gen_map_it_get_next_(struct map_T *map, struct map_it_T *it) {
  assert(map && it);

  if (it->index >= map->b_num - 1) {
    ERROR("Map iterator exceeded map size.");
    return;
  }
  if (it->current && it->current->next) {
    it->current = it->current->next;
    return;
  }

  it->current = NULL;
  while (++it->index < map->b_num) {
    if (map->bucket[it->index]) {
      it->current = map->bucket[it->index];
      break;
    }
  }
};

s32 gen_map_it_is_end(struct map_T *map, struct map_it_T *it) {
  assert(map && it);
  if (map->b_num - 1 >= it->index && it->current == NULL)
    return 1;
  else
    return 0;
};

void gen_map_it_set_begin(struct map_T *map, struct map_it_T *it) {
  assert(map && it);
  it->current = map->bucket[0];
  it->index = 0;
  if (!it->current)
    gen_map_it_get_next_(map, it);
};

void gen_map_it_get_next(struct map_T *map, struct map_it_T *it) {
  assert(map && it);
  gen_map_it_get_next_(map, it);
};
