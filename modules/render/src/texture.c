#include "glad/gl.h"

#include "texture.h"

#include <assert.h>
#include <data/serial.h>
#include <stdlib.h>

struct texture_T textures[MAX_TEXTURES];

static struct serial_T *serial;

s32 ren_create_texture(u8 *bitmap, s32 width, s32 height, s32 channels,
                       enum IMAGE_TYPE type) {

  if (serial == NULL) {
    serial = gen_serial_create_from(1);
  }

  assert(serial != NULL);
  s32 id;
  if (gen_serial_stamp(serial, &id) != 0)
    return -1;

  unsigned int texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // TODO: Lack of a switch statment for 'type' to load diferent types of
  // immages.

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, bitmap);
  glGenerateMipmap(GL_TEXTURE_2D);

  textures[id].gl_id = texture;

  return id;
}
