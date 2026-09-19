#include <glad/gl.h>

#include <math.h>

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
static s32 gl_enabled_ = 1;

static s32 sky_object_ = 0;
static s32 sky_program_ = 0;
static f32 sky_top_[3] = {0.16f, 0.24f, 0.38f};
static f32 sky_bottom_[3] = {0.03f, 0.04f, 0.06f};
static s32 sky_stiff_ = 2000;

void ren_set_gl_enabled(s32 enabled) { gl_enabled_ = enabled; }

void ren_skybox_set(u32 top_hex, u32 bottom_hex, s32 stiffness) {
  sky_top_[0] = ((top_hex >> 16) & 0xFF) / 255.0f;
  sky_top_[1] = ((top_hex >> 8) & 0xFF) / 255.0f;
  sky_top_[2] = (top_hex & 0xFF) / 255.0f;
  sky_bottom_[0] = ((bottom_hex >> 16) & 0xFF) / 255.0f;
  sky_bottom_[1] = ((bottom_hex >> 8) & 0xFF) / 255.0f;
  sky_bottom_[2] = (bottom_hex & 0xFF) / 255.0f;
  if (stiffness >= 0) {
    sky_stiff_ = stiffness;
  }
}

static void ren_draw_skybox_(mat4 view) {
  if (sky_program_ == 0) {
    sky_program_ = ren_create_program_from_files(
        "skybox", SHADERS_SOURCE_DIR "skybox_v.glsl",
        SHADERS_SOURCE_DIR "skybox_f.glsl");
  }
  if (sky_object_ == 0) {
    sky_object_ = ren_primitive_create_hud_plane(NULL);
  }
  struct program_T sky_p = programs[sky_program_];
  struct object_T sky_o = objects[sky_object_];

  glDisable(GL_DEPTH_TEST);
  glUseProgram(sky_p.id);

  vec3 top = {sky_top_[0], sky_top_[1], sky_top_[2]};
  vec3 bottom = {sky_bottom_[0], sky_bottom_[1], sky_bottom_[2]};
  f32 sn = sky_stiff_ / 4000.0f;
  ren_program_set_vec3(sky_p.id, "u_top", top);
  ren_program_set_vec3(sky_p.id, "u_bottom", bottom);
  ren_program_set_f32(sky_p.id, "u_morph", sn * sn);
  ren_program_set_mat4(sky_p.id, "u_view", view);
  ren_program_set_f32(sky_p.id, "u_tan_half_fov", tanf(main_camera.fov * 0.5f));
  ren_program_set_f32(sky_p.id, "u_aspect", main_camera.aspect_ratio);

  glBindVertexArray(sky_o.VAO);
  glDrawArrays(GL_TRIANGLES, 0, sky_o.n_triangles * 3);

  if (active_buffers.depth) {
    glEnable(GL_DEPTH_TEST);
  }
}

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

  if (gl_enabled_) {
    glEnable(GL_SCISSOR_TEST);
    glScissor(ren_viewport_.x, ren_viewport_.y, ren_viewport_.w,
              ren_viewport_.h);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);
    glViewport(ren_viewport_.x, ren_viewport_.y, ren_viewport_.w,
               ren_viewport_.h);

    if (active_buffers.depth) {
      glEnable(GL_DEPTH_TEST);
    }
  }
};

s8 ren_draw_frame() {
  ren_clean_screen_();

  mat4 view;
  ren_get_view_matrix(view);

  mat4 projection;
  ren_get_projection_matrix(&projection);

  s32 i;

  ren_draw_ui_frame_();

  if (!gl_enabled_) {
    return 0;
  }

  ren_draw_skybox_(view);

  // TODO: SAFETY CHECK AND REDESING ENTIRELY
  for (i = 0; i < MAX_ENTITIES; i++) {
    struct entity_T e = entities[i];

    if (e.id == 0) {
      continue;
    }

    s32 m_id = e.components[COMPONENT_MATERIAL];
    if (materials[m_id].texture == 0 && materials[m_id].program == 0) {
      INFO("Material has not been set for entity %d", e.id);
      continue;
    }

    s32 p_id = materials[m_id].program;
    if (p_id <= 0 || p_id >= MAX_PROGRAMS || !programs[p_id].id)
      continue;
    s32 p_gl_id = programs[p_id].id;

    glUseProgram(p_gl_id);

    ren_program_set_mat4(p_gl_id, "projection", projection);

    ren_program_set_mat4(p_gl_id, "view", view);

    u32 o_id = e.components[COMPONENT_OBJECT];
    mat4 model;
    ren_get_model_mat(o_id, model);
    ren_program_set_mat4(p_gl_id, "model", model);

    // i need parser and material utils asap
    struct object_T o = objects[o_id];
    s32 t_id = materials[m_id].texture;
    if (materials[m_id].has_color)
      ren_program_set_vec3(p_gl_id, "u_color", materials[m_id].color);
    else {
      vec3 def = {1.0f, 1.0f, 1.0f};
      ren_program_set_vec3(p_gl_id, "u_color", def);
    }
    if (materials[m_id].line_width > 0.0f)
      ren_program_set_f32(p_gl_id, "u_thickness", materials[m_id].line_width);
    else
      ren_program_set_f32(p_gl_id, "u_thickness", 1.0f);
    {
      vec2 vp = {(f32)ren_viewport_.w, (f32)ren_viewport_.h};
      ren_program_set_vec2(p_gl_id, "u_viewport", vp);
    }
    ren_program_set_f32(p_gl_id, "u_alpha",
                        materials[m_id].transparent ? 0.6f : 1.0f);
    if (t_id > 0 && t_id < MAX_TEXTURES && textures[t_id].gl_id)
      glBindTexture(GL_TEXTURE_2D, textures[t_id].gl_id);
    if (materials[m_id].transparent) {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    glBindVertexArray(o.VAO);
    if (o.indexed && o.EBO) {
      GLenum mode = o.primitive ? o.primitive : GL_TRIANGLES;
      glDrawElements(mode, o.n_indices, GL_UNSIGNED_INT, 0);
    } else if (o.primitive && o.primitive != GL_TRIANGLES) {
      glDrawArrays(o.primitive, 0, o.n_vertices);
    } else {
      glDrawArrays(GL_TRIANGLES, 0, o.n_triangles * 3);
    }
    if (materials[m_id].transparent) {
      glDisable(GL_BLEND);
    }
  }

  return 0;
}

void ren_set_viewport(struct rect_T bound) { ren_viewport_ = bound; };

void ren_set_ui_background(s32 texture_id, struct rect_T bound) {

  ui_texture_ = texture_id;
  ui_viewport_ = bound;

  if (ui_object_ == 0) {
    ui_object_ = ren_primitive_create_hud_plane(NULL);
  }

  if (ui_program_ == 0) {
    ui_program_ =
        ren_create_program_from_files("cef", SHADERS_SOURCE_DIR "cef_v.glsl",
                                      SHADERS_SOURCE_DIR "cef_f.glsl");
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

void ren_update_ui_background_sub(struct rect_T dst, u8 *bitmap) {
  if (ui_texture_ == 0)
    return;
  GLuint gl_id = textures[ui_texture_].gl_id;
  if (gl_id == 0)
    return;
  glBindTexture(GL_TEXTURE_2D, gl_id);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, dst.w);
  glTexSubImage2D(GL_TEXTURE_2D, 0, dst.x, dst.y, dst.w, dst.h, GL_BGRA,
                  GL_UNSIGNED_BYTE, bitmap);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  glBindTexture(GL_TEXTURE_2D, 0);
};
