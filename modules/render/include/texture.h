#ifndef TEXTURE_H
#define TEXTURE_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <types.h>

struct texture_T {
  u32 gl_id;
};

#define MAX_TEXTURES 1024
extern struct texture_T textures[MAX_TEXTURES];

enum IMAGE_TYPE { RGB = 0, RGBA };

extern s32 ren_create_texture(u8 *bitmap, s32 width, s32 height,
                              s32 channels, enum IMAGE_TYPE type);

extern s32 ren_delete_texture(s32 id);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //! TEXTURE_H
