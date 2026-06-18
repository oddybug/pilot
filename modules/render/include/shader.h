#ifndef SHADER_H
#define SHADER_H

#ifdef __cplusplus
extern "C" {
#endif //  __cplusplus

#include <cglm/cglm.h>

#include <types.h>

enum SHADER_TYPE { RENDER_VERTEX_SHADER = 0, RENDER_FRAGEMENT_SHADER };

/**
 * @brief creates shader of type 'type' with source code 'src'. Note that you
 * have to manually call OpenGL API calls to load these shaders into program and
 * manualy delete shaders after.
 *
 * @param type
 * @param src
 * @return the id of the shader created
 */
extern s32 ren_create_shader(enum SHADER_TYPE type, const s8 *src);

/**
 * @brief creates shader program using fragments 'fragment_src' and vertexs
 * 'vertex_src' source code.
 *
 * @param vertex
 * @param fragment
 * @return programs ID on succes and -1 on failure
 */
extern s32 ren_create_program(const s8 *vertex_src, const s8 *fragment_src);

/**
 * @brief bind hader program with id 'id'
 *
 * @param id
 */
extern void ren_bind_program(s32 id);

// void setBool(s32 id, const char *name, bool value);

/**
 * @brief set uniform value 'name' to shader program with id 'id' and value
 * 'value'
 *
 * @param id
 * @param name
 * @param value
 */
extern void ren_shader_set_s32(s32 id, const char *name, s32 value);

/**
 * @brief set uniform value 'name' to shader program with id 'id' and value
 * 'value'
 *
 * @param id
 * @param name
 * @param value
 */
extern void ren_shader_set_f32(s32 id, const char *name, f32 value);

/**
 * @brief set uniform value 'name' to shader program with id 'id' and value
 * 'value'
 *
 * @param id
 * @param name
 * @param value
 */
extern void ren_shader_set_vec2(s32 id, const char *name, vec2 value);

/**
 * @brief set uniform value 'name' to shader program with id 'id' and value
 * 'value'
 *
 * @param id
 * @param name
 * @param value
 */
extern void ren_shader_set_vec3(s32 id, const char *name, vec3 value);

/**
 * @brief set uniform value 'name' to shader program with id 'id' and value
 * 'value'
 *
 * @param id
 * @param name
 * @param value
 */
extern void ren_shader_set_mat4(s32 id, const char *name, const mat4 value);

#ifdef __cplusplus
}
#endif //  __cplusplus

#endif // !SHADER_H
