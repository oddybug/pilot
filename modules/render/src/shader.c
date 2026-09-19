#include <errno.h>
#include <glad/gl.h>

#include <assert.h>
#include <stdio.h>

#include "data/atom.h"
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

static s32 _ren_create_geometry_shader(const char *src) {
  s32 geom = glCreateShader(GL_GEOMETRY_SHADER);
  s32 success;
  GLchar info[512];
  glShaderSource(geom, 1, &src, NULL);
  glCompileShader(geom);
  glGetShaderiv(geom, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(geom, 512, NULL, info);
    fprintf(stderr, "Geometry shader compilation failed: %s\n", info);
    return -1;
  }
  return geom;
};

s32 ren_create_shader(enum SHADER_TYPE type, const char *src) {
  s32 id = -1;
  switch (type) {
  case RENDER_VERTEX_SHADER:
    id = _ren_create_vertex_shader(src);
    break;
  case RENDER_FRAGEMENT_SHADER:
    id = _ren_create_fragment_shader(src);
    break;
  case RENDER_GEOMETRY_SHADER:
    id = _ren_create_geometry_shader(src);
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

static s32 _ren_create_program_with_gs(s32 vertex, s32 geom, s32 fragment);

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

static s32 _ren_create_program_with_gs(s32 vertex, s32 geom, s32 fragment) {
  s32 success;
  GLchar info[512];
  s32 id = glCreateProgram();
  glAttachShader(id, vertex);
  glAttachShader(id, geom);
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

s32 ren_create_program(const c8 *name, const char *vertex_src,
                       const char *fragment_src) {
  s32 fragment = _ren_create_fragment_shader(fragment_src);
  if (fragment == -1) {
    return fragment;
  }

  s32 vertex = _ren_create_vertex_shader(vertex_src);

  if (vertex == -1) {
    glDeleteShader(fragment);
    return vertex;
  }

  s32 p_gl_id = _ren_create_program(vertex, fragment);

  if (serial == NULL) {
    serial = gen_serial_create_from(1);
  }

  assert(serial != NULL);

  s32 id;
  if (gen_serial_stamp(serial, &id) != 0) {
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    if (p_gl_id != -1)
      glDeleteProgram(p_gl_id);
    return -1;
  }

  glDeleteShader(vertex);
  glDeleteShader(fragment);

  if (p_gl_id == -1) {
    return p_gl_id;
  }

  programs[id].vs_id = vertex;
  programs[id].gs_id = 0;
  programs[id].fs_id = fragment;
  programs[id].id = p_gl_id;
  programs[id].name = name && name[0] ? gen_atom(name) : NULL;

  return id;
};

s32 ren_create_program_with_geometry(const c8 *name, const char *vertex_src,
                                     const char *geometry_src,
                                     const char *fragment_src) {
  s32 vertex = _ren_create_vertex_shader(vertex_src);
  if (vertex == -1)
    return -1;
  s32 geom = _ren_create_geometry_shader(geometry_src);
  if (geom == -1) {
    glDeleteShader(vertex);
    return -1;
  }
  s32 fragment = _ren_create_fragment_shader(fragment_src);
  if (fragment == -1) {
    glDeleteShader(vertex);
    glDeleteShader(geom);
    return -1;
  }
  s32 p_gl_id = _ren_create_program_with_gs(vertex, geom, fragment);
  if (serial == NULL) {
    serial = gen_serial_create_from(1);
  }
  assert(serial != NULL);
  s32 id;
  if (gen_serial_stamp(serial, &id) != 0) {
    glDeleteShader(vertex);
    glDeleteShader(geom);
    glDeleteShader(fragment);
    if (p_gl_id != -1)
      glDeleteProgram(p_gl_id);
    return -1;
  }
  glDeleteShader(vertex);
  glDeleteShader(geom);
  glDeleteShader(fragment);
  if (p_gl_id == -1) {
    return -1;
  }
  programs[id].vs_id = vertex;
  programs[id].gs_id = geom;
  programs[id].fs_id = fragment;
  programs[id].id = p_gl_id;
  programs[id].name = name && name[0] ? gen_atom(name) : NULL;
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

s32 ren_create_program_from_files(const c8 *name, const char *vertex_src_dir,
                                  const char *fragment_src_dir) {
  const char *const v_src = ren_file_to_str(vertex_src_dir);
  const char *const f_src = ren_file_to_str(fragment_src_dir);
  if (!v_src || !f_src) {
    if (v_src)
      free((void *)v_src);
    if (f_src)
      free((void *)f_src);
    return -1;
  }
  s32 id = ren_create_program(name, v_src, f_src);
  free((void *)v_src);
  free((void *)f_src);
  return id;
};

s32 ren_create_program_from_files_with_geometry(const c8 *name,
                                                const char *vertex_src_dir,
                                                const char *geometry_src_dir,
                                                const char *fragment_src_dir) {
  const char *const v_src = ren_file_to_str(vertex_src_dir);
  const char *const g_src = ren_file_to_str(geometry_src_dir);
  const char *const f_src = ren_file_to_str(fragment_src_dir);
  if (!v_src || !g_src || !f_src) {
    if (v_src)
      free((void *)v_src);
    if (g_src)
      free((void *)g_src);
    if (f_src)
      free((void *)f_src);
    return -1;
  }
  s32 id = ren_create_program_with_geometry(name, v_src, g_src, f_src);
  free((void *)v_src);
  free((void *)g_src);
  free((void *)f_src);
  return id;
};

s32 ren_delete_program(u32 program) {
  if (programs[program].id == 0) {
    ERROR("Program with id %d not initialized", program);
    return 1;
  }
  if (programs[program].gs_id)
    glDeleteShader(programs[program].gs_id);
  if (programs[program].fs_id)
    glDeleteShader(programs[program].fs_id);
  if (programs[program].vs_id)
    glDeleteShader(programs[program].vs_id);
  glDeleteProgram(programs[program].id);
  programs[program].id = 0;
  programs[program].vs_id = 0;
  programs[program].gs_id = 0;
  programs[program].fs_id = 0;
  programs[program].name = NULL;
  return 0;
};

void ren_bind_program(s32 id) { glUseProgram(id); };

void ren_program_set_s32(s32 id, const char *name, s32 value) {
  glUniform1i(glGetUniformLocation(id, name), value);
};

void ren_program_set_f32(s32 id, const char *name, f32 value) {
  glUniform1f(glGetUniformLocation(id, name), value);
};

void ren_program_set_vec2(s32 id, const char *name, vec2 value) {
  glUniform2f(glGetUniformLocation(id, name), value[0], value[1]);
};

void ren_program_set_vec3(s32 id, const char *name, vec3 value) {
  glUniform3f(glGetUniformLocation(id, name), value[0], value[1], value[2]);
};

void ren_program_set_mat4(s32 id, const char *name, const mat4 value) {
  glUniformMatrix4fv(glGetUniformLocation(id, name), 1, GL_FALSE, &value[0][0]);
};
