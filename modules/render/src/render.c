#include <GLES2/gl2.h>
#include <glad/gl.h>

#include <render.h>

#include "camera.h"
#include "entity.h"
#include "log.h"
#include "object.h"
#include "shader.h"

struct active_buffers_T {
  u8 color;
  u8 depth;
  u8 stencil;
};

static struct active_buffers_T active_buffers = {
    .color = 0, .depth = 0, .stencil = 0};

/**
 * @brief Clean all OpenGL buffers that are active.
 */
static void ren_clean_screen(void);

static void ren_clean_screen(void) {

  u32 flag = 0;

  if (active_buffers.color) {
    flag |= GL_COLOR_BUFFER_BIT;
  }

  if (active_buffers.depth) {
    flag |= GL_DEPTH_BUFFER_BIT;
  }

  if (active_buffers.stencil) {
    flag |= GL_STENCIL_BUFFER_BIT;
  }

  glClearColor(0.8f, 0.8f, 0.95f, 1.0f);
  glClear(flag);
};

s32 ren_init() {
  active_buffers.color = 1;
  active_buffers.depth = 1;

  glEnable(GL_DEPTH_TEST);
  return 0;
};

s8 ren_draw_frame() {
  ren_clean_screen();

  mat4 view;
  ren_get_view_matrix(view);

  mat4 projection;
  ren_get_projection_matrix(&projection);

  s32 i;

  // TODO: SAFETY CHECK AND REDESING ENTIRELY

  for (i = 0; i < MAX_ENTITIES; i++) {
    struct entity_T e = entities[i];

    if (e.id == 0) {
      continue;
    }

    s32 p_id = e.components[COMPONENT_PROGRAM];
    s32 p_gl_id = programs[p_id].id;
    glUseProgram(p_gl_id);

    ren_program_set_mat4(p_gl_id, "projection", projection);

    ren_program_set_mat4(p_gl_id, "view", view);

    u32 o_id = e.components[COMPONENT_OBJECT];
    mat4 model;
    ren_get_model_mat(o_id, model);
    ren_program_set_mat4(p_gl_id, "model", model);

    struct object_T o = objects[o_id];
    glBindVertexArray(o.VAO);
    glDrawArrays(GL_TRIANGLES, 0, o.n_triangles * 3);
  }

  // for e in entities{
  // use pragram from entity
  // update in and uniforms
  // draw
  // }

  return 0;
}
