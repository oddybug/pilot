#ifndef MATERIAL_H
#define MATERIAL_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "cglm/types.h"
#include "types.h"

struct material_T {
  s32 program;
  s32 texture;
  vec3 color;
  s32 has_color;
  s32 transparent;
  f32 line_width;
  const c8 *name;
};

#define MAX_MATERIALS 1024
extern struct material_T materials[MAX_MATERIALS];

s32 ren_create_material(const c8 *name);

s32 ren_material_set_texture(s32 m_id, s32 t_id);

s32 ren_material_set_program(s32 m_id, s32 p_id);

// temp
void ren_material_set_color(s32 m_id, vec3 color);

// temp
void ren_material_set_transparent(s32 m_id, s32 transparent);

// temp
void ren_material_set_line_width(s32 m_id, f32 width);

struct material_T ren_get_material(s32 id);

// temp?
s32 ren_delete_material(s32 id);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !MATERIAL_H
