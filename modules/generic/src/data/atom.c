#include "data/atom.h"

#include <stdlib.h>
#include <string.h>

#include "data/hashmap.h"
#include "data/hashmap_helpers.h"
#include "types.h"

static map_T g_atoms = NULL;

const c8 *gen_atom(const c8 *str) {
  if (!str)
    return NULL;
  if (!g_atoms) {
    g_atoms =
        gen_map_create(64, gen_map_hash_fn_c8p, gen_map_cmp_key_c8p, NULL);
    if (!g_atoms)
      return NULL;
  }
  void *found = gen_map_find(g_atoms, str);
  if (found)
    return (const c8 *)found;
  size_t n = strlen(str) + 1;
  c8 *copy = (c8 *)malloc(n);
  c8 *val = (c8 *)malloc(n);
  if (!copy || !val) {
    free(copy);
    free(val);
    return NULL;
  }
  memcpy(copy, str, n);
  memcpy(val, str, n);
  if (gen_map_insert(g_atoms, copy, val) != 0) {
    free(copy);
    free(val);
    return NULL;
  }
  return val;
}

void gen_atom_free(void) {
  if (!g_atoms)
    return;
  gen_map_free(g_atoms);
  g_atoms = NULL;
}
