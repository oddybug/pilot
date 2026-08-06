#ifndef DMATH
#define DMATH

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

struct rect_T {
  s32 x, y;
  s32 w, h;
};

struct point_T {
  s32 x, y;
};

#ifdef __cplusplus
}
#endif
#endif // !DMATH
