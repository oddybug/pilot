#ifndef OBJECT_H
#define OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <types.h>

s32 init_objects_array();

s32 ren_primitive_create_cube();

s32 ren_delete_object(s32 id);


#ifdef __cplusplus
}
#endif // __cplusplus
#endif // !OBJECT_H
