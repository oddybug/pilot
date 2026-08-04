#ifndef TEXTURE_H
#define TEXTURE_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <types.h>

enum TEXTURE_TYPE { RGB = 0, RGBA, BGRA };

struct texture_T {
  u32 gl_id;
  u32 width;
  u32 height;
  enum TEXTURE_TYPE type;
};

#define MAX_TEXTURES 1024
extern struct texture_T textures[MAX_TEXTURES];

extern s32 ren_create_texture(u8 *bitmap, s32 width, s32 height, s32 channels,
                              enum TEXTURE_TYPE type);

extern s32 ren_update_texture_bitmap(s32 id, u8 *bitmap);

extern s32 ren_update_texture(s32 id, u8 *new_bitmap, s32 new_width,
                              s32 new_height);

extern s32 ren_delete_texture(s32 id);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //! TEXTURE_H
