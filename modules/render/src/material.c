#include "material.h"

#include <assert.h>
#include <cglm/cglm.h>
#include <data/atom.h>
#include <data/serial.h>

struct material_T materials[MAX_MATERIALS];

static struct serial_T *serial;

// TODO: in a near future if any component of the ECS is reused will have trash
// in it as it doesnt get cleaned on creation. (note how the first time is 0
// cause of the static initialitzation in C) !URGENT
s32 ren_create_material(const c8 *name) {

  if (serial == NULL) {
    serial = gen_serial_create_from(1);
  }

  assert(serial != NULL);
  s32 id;
  if (gen_serial_stamp(serial, &id) != 0)
    return -1;

  materials[id].name = name && name[0] ? gen_atom(name) : NULL;
  materials[id].program = 0;
  materials[id].texture = 0;
  materials[id].has_color = 0;
  materials[id].transparent = 0;
  materials[id].line_width = 0.0f;
  glm_vec3_zero(materials[id].color);
  return id;
};

s32 ren_material_set_texture(s32 m_id, s32 t_id) {
  materials[m_id].texture = t_id;
  return 0;
};

s32 ren_material_set_program(s32 m_id, s32 p_id) {
  materials[m_id].program = p_id;
  return 0;
};

void ren_material_set_color(s32 m_id, vec3 color) {
  glm_vec3_copy(color, materials[m_id].color);
  materials[m_id].has_color = 1;
};

void ren_material_set_transparent(s32 m_id, s32 transparent) {
  materials[m_id].transparent = transparent;
};

void ren_material_set_line_width(s32 m_id, f32 width) {
  materials[m_id].line_width = width;
};

struct material_T ren_get_material(s32 id) { return materials[id]; };

s32 ren_delete_material(s32 id) {
  if (id < 0 || id >= MAX_MATERIALS ||
      (materials[id].texture == 0 && !materials[id].has_color))
    return -1;
  materials[id].texture = 0;
  materials[id].program = 0;
  materials[id].has_color = 0;
  materials[id].transparent = 0;
  materials[id].line_width = 0.0f;
  glm_vec3_zero(materials[id].color);
  materials[id].name = NULL;
  return 0;
}
