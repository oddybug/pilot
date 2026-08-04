#ifndef MATERIAL_H
#define MATERIAL_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "types.h"

struct material_T {
  s32 texture;
};

#define MAX_MATERIALS 1024
extern struct material_T materials[MAX_MATERIALS];

extern s32 ren_create_material();

extern s32 ren_material_set_texture(s32 m_id, s32 t_id);

struct material_T ren_get_material(s32 id);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !MATERIAL_H
