#include <glad/gl.h>

#include <render.h>

#include "camera.h"
#include "entity.h"
#include "log.h"
#include "material.h"
#include "object.h"
#include "shader.h"
#include "texture.h"

struct active_buffers_T {
  u8 color;
  u8 depth;
  u8 stencil;
};

static struct active_buffers_T active_buffers = {
    .color = 0, .depth = 0, .stencil = 0};

static struct rect_T ren_viewport_;

static struct rect_T ui_viewport_;
static s32 ui_object_;
static s32 ui_program_;
static s32 ui_texture_;

/**
 * @brief Clean all OpenGL buffers that are active.
 */
static void ren_clean_screen_(void);

static void ren_clean_screen_(void) {

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

  glClear(flag);

  glClearColor(0.8f, 0.8f, 0.95f, 1.0f);
};

s32 ren_init() {
  active_buffers.color = 1;
  active_buffers.depth = 1;

  glEnable(GL_DEPTH_TEST);
  return 0;
};

static void ren_draw_ui_frame_();

static void ren_draw_ui_frame_() {
  struct object_T ui_o = objects[ui_object_];
  struct program_T ui_p = programs[ui_program_];

  glViewport(ui_viewport_.x, ui_viewport_.y, ui_viewport_.w, ui_viewport_.h);
  glDisable(GL_DEPTH_TEST);

  glUseProgram(ui_p.id);

  glBindTexture(GL_TEXTURE_2D, textures[ui_texture_].gl_id);
  glBindVertexArray(ui_o.VAO);
  glDrawArrays(GL_TRIANGLES, 0, ui_o.n_triangles * 3);

  glEnable(GL_SCISSOR_TEST);
  glScissor(ren_viewport_.x, ren_viewport_.y, ren_viewport_.w, ren_viewport_.h);
  glClear(GL_COLOR_BUFFER_BIT);
  glDisable(GL_SCISSOR_TEST);
  glViewport(ren_viewport_.x, ren_viewport_.y, ren_viewport_.w,
             ren_viewport_.h);
};

s8 ren_draw_frame() {
  ren_clean_screen_();

  mat4 view;
  ren_get_view_matrix(view);

  mat4 projection;
  ren_get_projection_matrix(&projection);

  s32 i;

  ren_draw_ui_frame_();

  // TODO: SAFETY CHECK AND REDESING ENTIRELY
  for (i = 0; i < MAX_ENTITIES; i++) {
    struct entity_T e = entities[i];

    if (e.id == 0) {
      continue;
    }

    s32 m_id = e.components[COMPONENT_MATERIAL];
    if (materials[m_id].texture == 0) {
      INFO("Material has not been set for entity %d", e.id);
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
    s32 t_id = materials[m_id].texture;
    glBindTexture(GL_TEXTURE_2D, textures[t_id].gl_id);
    glBindVertexArray(o.VAO);
    glDrawArrays(GL_TRIANGLES, 0, o.n_triangles * 3);
  }

  return 0;
}

void ren_set_viewport(struct rect_T bound) { ren_viewport_ = bound; };

void ren_set_ui_background(s32 texture_id, struct rect_T bound) {

  ui_texture_ = texture_id;
  ui_viewport_ = bound;

  if (ui_object_ == 0) {
    ui_object_ = ren_primitive_create_hud_plane();
  }

  if (ui_program_ == 0) {
    ui_program_ = ren_create_program_from_files(
        SHADERS_SOURCE_DIR "cef_v.glsl", SHADERS_SOURCE_DIR "cef_f.glsl");
  }
};

void ren_update_ui_background_bitmap(u8 *bitmap) {
  if (ui_texture_ == 0) {
    ERROR("ui texture has not been initialized");
    return;
  }
  ren_update_texture(ui_texture_, bitmap, ui_viewport_.w, ui_viewport_.h);
};

void ren_update_ui_background(struct rect_T bound, u8 *bitmap) {
  if (ui_texture_ == 0) {
    ERROR("ui texture has not been initialized");
    return;
  }

  ui_viewport_ = bound;
  ren_update_texture(ui_texture_, bitmap, bound.w, bound.h);
};
