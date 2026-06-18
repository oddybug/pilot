#include <glad/gl.h>

#include <stdio.h>

#include "shader.h"

/**
 * @brief create fragment shader with source code 'src'
 *
 * @param src
 * @return id of the shader, returns -1 if failed to create shader
 */
s32 _ren_create_fragment_shader(const s8 *src);

s32 _ren_create_fragment_shader(const s8 *src) {

  s32 fragment = glCreateShader(GL_VERTEX_SHADER);

  s32 success;
  GLchar info[512];

  glShaderSource(fragment, 1, (const GLchar **)&src, NULL);
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
s32 _ren_create_vertex_shader(const s8 *src);

s32 _ren_create_vertex_shader(const s8 *src) {

  s32 vertex = glCreateShader(GL_VERTEX_SHADER);

  s32 success;
  GLchar info[512];

  glShaderSource(vertex, 1, (const GLchar **)&src, NULL);
  glCompileShader(vertex);

  glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertex, 512, NULL, info);
    fprintf(stderr, "Vertex fragment compilation failed: %s\n", info);
  }

  return vertex;
};

s32 ren_create_shader(enum SHADER_TYPE type, const s8 *src) {
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
 * @brief create shader program with vertexs 'vertex' and fragment 'fragment' id's
 *
 * @param vertex 
 * @param fragment 
 * @return 
 */
s32 _ren_create_program(s32 vertex, s32 fragment);

s32 _ren_create_program(s32 vertex, s32 fragment) {

  s32 success;
  GLchar info[512];

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

s32 ren_create_program(const s8 *vertex_src, const s8 *fragment_src) {
  s32 fragment = _ren_create_fragment_shader(fragment_src);
  if (fragment == -1) {
    return fragment;
  }

  s32 vertex = _ren_create_vertex_shader(vertex_src);

  if (vertex == -1) {
    return vertex;
  }

  s32 id = _ren_create_program(vertex, fragment);

  glDeleteShader(vertex);
  glDeleteShader(fragment);

  if (id == -1) {
    return id;
  }

  return id;
};

void ren_bind_program(s32 id) { glUseProgram(id); };

void ren_shader_set_s32(s32 id, const char *name, s32 value) {
  glUniform1i(glGetUniformLocation(id, name), value);
};

void ren_shader_set_f32(s32 id, const char *name, f32 value) {
  glUniform1f(glGetUniformLocation(id, name), value);
};

void ren_shader_set_vec2(s32 id, const char *name, vec2 value) {
  glUniform2f(glGetUniformLocation(id, name), value[0], value[1]);
};

void ren_shader_set_vec3(s32 id, const char *name, vec3 value) {
  glUniform3f(glGetUniformLocation(id, name), value[0], value[1], value[2]);
};

void ren_shader_set_mat4(s32 id, const char *name, const mat4 value) {
  glUniformMatrix4fv(glGetUniformLocation(id, name), 1, GL_FALSE, &value[0][0]);
};
