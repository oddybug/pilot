#ifndef OBJECT_H
#define OBJECT_H

#include "cglm/types.h"
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <types.h>

struct object_T {
  u32 VBO[2];
  u32 VAO;

  u32 id;

  u32 n_triangles;

  vec3 position;
  vec3 rotation;
};

#define MAX_OBJECTS 1024
extern struct object_T objects[MAX_OBJECTS];

s32 ren_primitive_create_cube();

s32 ren_delete_object(s32 id);

// TODO: maybe this object is not active. Needs support runtime checked error.
void ren_get_model_mat(s32 id, mat4 model);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // !OBJECT_H
