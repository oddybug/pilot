#include "data/hashmap_helpers.h"
#include <assert.h>
#include <string.h>

#include "log.h"

u32 gen_map_cmp_key_c8p(const void *a, const void *b) {
  assert(a && b);
  if (!strcmp((c8 *)a, (c8 *)b))
    return 1;
  else
    return 0;
};

u32 gen_map_hash_fn_c8p(const void *key) {
  assert(key);

  if (!key) {
    ERROR("provided key is NULL");
    return 0;
  }

  u32 hash = 5381;
  c8 c;

  while ((c = *(c8 *)key++)) {
    hash = ((hash << 5) + hash) + (u32)c;
  }

  return hash;
};
