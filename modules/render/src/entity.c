#include "log.h"
#include <assert.h>
#include <entity.h>

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>

#include <data/serial.h>

static struct serial_T *serial;

#define MAX_ENTITIES 1024

struct entity_T entities[MAX_ENTITIES];

extern u32 ren_create_entity() {
  if (serial == NULL) {
    serial = gen_serial_create_from(1);
  }

  assert(serial != NULL);

  s32 id;
  s32 err = gen_serial_stamp(serial, &id);

  if (err != 0)
    return 0;

  entities[id].id = id;
  return id;
}

extern u32 get_entity_id(u32 entity) {
  assert(entity != 0);
  return entities[entity].id;
}

extern void ren_entity_add_component(u32 entity, enum COMPONENT_TYPE type,
                                     u32 id) {
  assert(entity != 0);
  entities[entity].components[type] = id;
}

extern s32 ren_entity_get_component(u32 entity, enum COMPONENT_TYPE type) {
  assert(entity != 0);
  return entities[entity].components[type];
}
