#include <errno.h>
#include <glad/gl.h>

#include <stdio.h>
#include <assert.h>

#include "data/serial.h"
#include "log.h"
#include "shader.h"

struct program_T programs[MAX_PROGRAMS];

static struct serial_T *serial;

/**
 * @brief create fragment shader with source code 'src'
 *
 * @param src
 * @return id of the shader, returns -1 if failed to create shader
 */
static s32 _ren_create_fragment_shader(const char *src);

static s32 _ren_create_fragment_shader(const char *src) {

  s32 fragment = glCreateShader(GL_FRAGMENT_SHADER);

  s32 success;
  GLchar info[512];

  glShaderSource(fragment, 1, &src, NULL);
  glCompileShader(fragment);

  glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragment, 512, NULL, info);
    fprintf(stderr, "Fragment shader compilation failed: %s\n", info);
    return -1;
  }

  return fragment;
}

/**
 * @brief create vertex shader with source code 'src'
 *
 * @param src
 * @return id of the shader, returns -1 if failed to create shader
 */
static s32 _ren_create_vertex_shader(const char *src);

static s32 _ren_create_vertex_shader(const char *src) {

  s32 vertex = glCreateShader(GL_VERTEX_SHADER);

  s32 success;
  GLchar info[512];

  glShaderSource(vertex, 1, &src, NULL);
  glCompileShader(vertex);

  glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertex, 512, NULL, info);
    fprintf(stderr, "Vertex fragment compilation failed: %s\n", info);
  }

  return vertex;
};

s32 ren_create_shader(enum SHADER_TYPE type, const char *src) {
  s32 id;
  switch (type) {
  case RENDER_VERTEX_SHADER:
    id = _ren_create_vertex_shader(src);
    break;
  case RENDER_FRAGEMENT_SHADER:
    id = _ren_create_fragment_shader(src);
    break;
  default:
    break;
  }

  return id;
};

/**
 * @brief create shader program with vertexs 'vertex' and fragment 'fragment'
 * id's
 *
 * @param vertex
 * @param fragment
 * @return returns OpenGL programs ID
 */
static s32 _ren_create_program(s32 vertex, s32 fragment);

static s32 _ren_create_program(s32 vertex, s32 fragment) {

  s32 success;
  GLchar info[512];

  INFO("vertex shader id: %d", vertex);
  INFO("fragment shader id: %d", fragment);

  s32 id = glCreateProgram();
  glAttachShader(id, vertex);
  glAttachShader(id, fragment);
  glLinkProgram(id);

  glGetProgramiv(id, GL_LINK_STATUS, &success);

  if (!success) {
    glGetProgramInfoLog(id, 512, NULL, info);
    fprintf(stderr, "Shader linking failed: %s\n", info);
    return -1;
  }

  return id;
};

extern s32 ren_create_program(const char *vertex_src,
                              const char *fragment_src) {
  s32 fragment = _ren_create_fragment_shader(fragment_src);
  if (fragment == -1) {
    return fragment;
  }

  s32 vertex = _ren_create_vertex_shader(vertex_src);

  if (vertex == -1) {
    return vertex;
  }

  s32 p_gl_id = _ren_create_program(vertex, fragment);

  if (serial == NULL) {
    serial = gen_serial_create_from(1);
  }

  assert(serial != NULL);

  s32 id;
  if (gen_serial_stamp(serial, &id) != 0)
    return -1;

  glDeleteShader(vertex);
  glDeleteShader(fragment);

  if (p_gl_id == -1) {
    return p_gl_id;
  }

  programs[id].fs_id = vertex;
  programs[id].fs_id = fragment;
  programs[id].id = p_gl_id;

  return id;
};

static char *ren_file_to_str(const char *dir) {

  FILE *f = fopen(dir, "rb");
  if (!f) {
    ERROR("File not found (errno: %d) - %s", errno, dir);
    return NULL;
  }

  if (fseek(f, 0, SEEK_END) == -1) {

    ERROR("Failed to seek in file (errno: %d) - %s", errno, dir);
    fclose(f);
    return NULL;
  }

  s64 size = ftell(f);

  rewind(f);

  // INFO: Stack could be used if load times got expensive. Heap is used
  // always for a simpler implementation.
  char *const src = malloc(sizeof(char) * size + 1);

  if (!src) {
    ERROR("Failed to malloc %d bytes when reading file (errno: %d) - %s", size,
          errno, dir);
    fclose(f);
    return NULL;
  }

  if (fread(src, sizeof(char), size, f) < size) {
    ERROR("Failed to read %d bytes when reading file (errno: %d) - %s", size,
          errno, dir);
    fclose(f);
    free(src);
    return NULL;
  };

  src[size] = '\0';

  return src;
};

extern s32 ren_create_program_from_files(const char *vertex_src_dir,
                                         const char *fragment_src_dir) {
  const char *const v_src = ren_file_to_str(vertex_src_dir);
  const char *const f_src = ren_file_to_str(fragment_src_dir);

  return ren_create_program(v_src, f_src);
};

extern s32 ren_delete_program(u32 program) {
  assert(programs[program].id != 0);
  assert(programs[program].fs_id != 0);
  assert(programs[program].vs_id != 0);

  if (programs[program].vs_id == 0 || programs[program].fs_id == 0 ||
      programs[program].id == 0) {
    ERROR("Program with id %d have wrong linkage to one or more of its "
          "shaders/programs",
          program);
    return 1;
  }

  glDeleteShader(programs[program].fs_id);
  programs[program].id = 0;
  glDeleteShader(programs[program].vs_id);
  programs[program].vs_id = 0;
  glDeleteProgram(programs[program].id);
  programs[program].fs_id = 0;

  return 0;
};

extern void ren_bind_program(s32 id) { glUseProgram(id); };

extern void ren_program_set_s32(s32 id, const char *name, s32 value) {
  glUniform1i(glGetUniformLocation(id, name), value);
};

extern void ren_program_set_f32(s32 id, const char *name, f32 value) {
  glUniform1f(glGetUniformLocation(id, name), value);
};

extern void ren_program_set_vec2(s32 id, const char *name, vec2 value) {
  glUniform2f(glGetUniformLocation(id, name), value[0], value[1]);
};

extern void ren_program_set_vec3(s32 id, const char *name, vec3 value) {
  glUniform3f(glGetUniformLocation(id, name), value[0], value[1], value[2]);
};

extern void ren_program_set_mat4(s32 id, const char *name, const mat4 value) {
  glUniformMatrix4fv(glGetUniformLocation(id, name), 1, GL_FALSE, &value[0][0]);
};
