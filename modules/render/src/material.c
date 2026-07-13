#include "material.h"

#include <assert.h>
#include <data/serial.h>
#include <stdlib.h>

struct material_T materials[MAX_MATERIALS];

static struct serial_T *serial;

// TODO: in a near future if any component of the ECS is reused will have trash
// in it as it doesnt get cleaned on creation. (note how the first time is 0
// cause of the static initialitzation in C) !URGENT
s32 ren_create_material() {

  if (serial == NULL) {
    serial = gen_serial_create_from(1);
  }

  assert(serial != NULL);
  s32 id;
  if (gen_serial_stamp(serial, &id) != 0)
    return -1;

  return id;
};

s32 ren_material_set_texture(s32 m_id, s32 t_id) {
  // return -1;
  materials[m_id].texture = t_id;
  return 0;
};
