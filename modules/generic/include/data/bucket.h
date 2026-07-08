#ifndef BUCKET_H

#include <types.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define T bucket_T

typedef struct T *T;

struct T {
  void *value;
  // void*
};

#undef T

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !BUCKET_H
