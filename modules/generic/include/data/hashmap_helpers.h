#ifndef HASHMAP_HELPERS_H
#define HASHMAP_HELPERS_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// #include "hashmap.h"
#include "types.h"

/**
 * @brief compare strings
 *
 * @param a
 * @param b
 * @return 1 if string equals and 0 otherwise
 */
extern u32 gen_map_cmp_key_c8p(const void *a, const void *b);

/**
 * @brief generates a hash from the string using djb2 algorithm
 *
 * @param key
 * @return
 */
extern u32 gen_map_hash_fn_c8p(const void *key);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !HASHMAP_HELPERS_H
