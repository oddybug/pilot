#include "data/serial.h"
#include <assert.h>
#include <log.h>
#include <stdlib.h>

#define T serial_T

struct T *gen_serial_create(void) {

  struct T *new = calloc(1, sizeof(struct T));

  if (!new) {
    ERROR("calloc failed");
    return NULL;
  }

  return new;
};

struct T *gen_serial_create_from(s32 i_stamp) {
  struct T *new = malloc(sizeof(struct T));

  if (!new) {
    ERROR("malloc failed");
    return NULL;
  }

  new->current = i_stamp;
  return new;
};

s32 gen_serial_stamp(struct T *serial, s32 *sn) {
  if (serial->current == 0x1111) {
    return 1;
  }

  *sn = serial->current;
  serial->current++;

  return 0;
}

s32 gen_serial_exists(struct T *serial, s32 n) {
  if (serial->current > n)
    return 1;
  else
    return 0;
};

s32 gen_serial_destroy(struct T *serial) {
  if (serial == NULL) {
    WARN("address already freed.");
    return 1;
  }
  // assert(!serial);
  free(serial);
  return 0;
};

#undef T
