#include "glad/gl.h"

#include "texture.h"

#include <GL/gl.h>
#include <assert.h>
#include <data/serial.h>
#include <stdlib.h>

struct texture_T textures[MAX_TEXTURES];

static struct serial_T *serial;

s32 ren_create_texture(u8 *bitmap, s32 width, s32 height, s32 channels,
                       enum TEXTURE_TYPE type) {

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

  switch (type) {
  case RGB:
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, bitmap);
  case RGBA:
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, bitmap);
  case BGRA:
    glTexImage2D(GL_TEXTURE_2D, 0, GL_BGRA, width, height, 0, GL_BGRA,
                 GL_UNSIGNED_BYTE, bitmap);
  default:
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, bitmap);

    break;
  }

  glGenerateMipmap(GL_TEXTURE_2D);

  textures[id].gl_id = texture;
  textures[id].width = width;
  textures[id].height = height;
  textures[id].type = type;

  return id;
}

s32 ren_update_texture_bitmap(s32 id, u8 *bitmap) {
  if (!bitmap || id < 0 || id >= MAX_TEXTURES) {
    return -1;
  }

  GLuint gl_id = textures[id].gl_id;
  if (gl_id == 0) {
    return -1;
  }

  s32 width = textures[id].width;
  s32 height = textures[id].height;
  GLenum format;

  switch (textures[id].type) {
  case RGB:
    format = GL_RGB;
  case RGBA:
    format = GL_RGBA;
  case BGRA:
    format = GL_BGRA;
  default:
    format = GL_RGBA;
    break;
  }

  glBindTexture(GL_TEXTURE_2D, gl_id);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, format,
                  GL_UNSIGNED_BYTE, bitmap);

  glGenerateMipmap(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D, 0);
  return 0;
}

s32 ren_update_texture(s32 id, u8 *new_bitmap, s32 new_width, s32 new_height) {
  if (new_width == 0 || new_height == 0 || id < 0 || id >= MAX_TEXTURES) {
    return -1;
  }

  glDeleteTextures(1, &textures[id].gl_id);

  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  GLenum internal_format = GL_RGBA8;
  GLenum format;

  switch (textures[id].type) {
  case RGB:
    format = GL_RGB;
  case RGBA:
    format = GL_RGBA;
  case BGRA:
    format = GL_BGRA;
  default:
    format = GL_RGBA;
    break;
  }

  glTexStorage2D(GL_TEXTURE_2D, 4, internal_format, new_width, new_height);

  if (new_bitmap) {
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, new_width, new_height, format,
                    GL_UNSIGNED_BYTE, new_bitmap);
    glGenerateMipmap(GL_TEXTURE_2D);
  }

  textures[id].gl_id = texture;
  textures[id].width = new_width;
  textures[id].height = new_height;

  return 0;
}

s32 ren_delete_texture(s32 id) {
  if (id < 0 || id >= MAX_TEXTURES || textures[id].gl_id == 0) {
    return -1;
  }

  glDeleteTextures(1, &textures[id].gl_id);

  textures[id].gl_id = 0;
  textures[id].width = 0;
  textures[id].height = 0;
  textures[id].type = 0;

  return 0;
}
