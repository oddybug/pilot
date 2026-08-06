#include "SDL3/SDL_events.h"

#include "dmath.h"
#include "ui.h"

extern "C" {
#include "camera.h"
#include "entity.h"
#include "global.h"
#include "io_manager.h"
#include "log.h"
#include "material.h"
#include "object.h"
#include "render.h"
#include "shader.h"
#include "texture.h"
#include "types.h"
}

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static s32 g_hud_m_id;

static s32 g_gl_target;
static s32 g_ui_target;

s32 create_texture(const char *file) {
  int width, height, nr_channels;
  u8 *data = stbi_load(file, &width, &height, &nr_channels, 0);
  if (data) {

    s32 id = ren_create_texture(data, width, height, nr_channels, RGBA);
    stbi_image_free(data);
    return id;
  } else {
    WARN("Error loading image: %s \n", file);
    stbi_image_free(data);
    return 0;
  }
}

void sdl_gl_callback(SDL_Event *e) { INFO("GL CALLBACK"); }

void sdl_ui_callback(SDL_Event *e) {
  INFO("UI CALLBACK");

  switch (e->type) {
  case SDL_EVENT_QUIT:
    state = 0;
    break;
  case SDL_EVENT_KEY_DOWN:
    switch (e->key.key) {
    case SDLK_ESCAPE:
      state = 0;
      break;
    default:
      break;
    }

    break;
  case SDL_EVENT_WINDOW_RESIZED: {
    s32 width = e->window.data1;
    s32 height = e->window.data2;
    INFO("window resized: (w: %d, h: %d)", width, height);
    ui_resize_window(width, height);

    struct rect_T b = {.x = 0, .y = 0, .w = width, .h = height};
    iom_resize_target(g_ui_target, b);

    break;
  }
  default:
    break;
  }
}

s32 create_ui_texture() {
  // TODO: temporary
  u8 *buffer = new u8[1280 * 720 * 4];
  for (int i = 0; i < 1280; i++) {
    for (int j = 0; j < 720; j++) {
      buffer[(j * 720 + i) * 4 + 0] = 255;
      buffer[(j * 720 + i) * 4 + 1] = 0;
      buffer[(j * 720 + i) * 4 + 2] = 0;
      buffer[(j * 720 + i) * 4 + 3] = 0;
    }
  }

  s32 id = ren_create_texture(buffer, 1280, 720, 4, BGRA);
  delete[] buffer;

  return id;
}

void start_camera() {
  ren_set_camera_aspect_ratio_wh(800, 600);
  ren_set_camera_planes(0.1f, 100.0f);
  ren_set_camera_projection(PERSPECTIVE);
  ren_set_camera_fov(1.3);
  ren_camera_set_position(0.0, 0.0, 5.0);
  ren_camera_set_rotation(0.0, 0.0, 0.0);
}

void ui_texture_clbk(u8 *bitmap, u32 width, u32 height) {
  INFO("callbacking");

  // TODO: think a clever way to do that. Temporal.
  // struct material_T mat = ren_get_material(g_hud_m_id);

  struct rect_T b = {
      .x = 0, .y = 0, .w = (s32)width, .h = (s32)height}; // Potential
                                                          // oberflow
                                                          // bug
  ren_update_ui_background(b, bitmap);
  // ren_update_texture(mat.texture, bitmap, width, height);
}

void create_targets() {

  g_gl_target = iom_create_target();
  g_ui_target = iom_create_target();

  struct rect_T b_ui = {.x = 0, .y = 0, .w = 1280, .h = 720}; // TODO: hardcoded
                                                              // !URGENT

  struct rect_T b_gl = {.x = 0, .y = 0, .w = 400, .h = 400}; // TODO: hardcoded
                                                             // !URGENT
  iom_set_target(g_ui_target, b_ui, 1, sdl_ui_callback);
  iom_set_target(g_gl_target, b_gl, 2, sdl_gl_callback);
}

int main(int argc, char *argv[]) {

  if (ui_start(argc, argv) != 0) {
    return -1;
  }

  ui_set_ui_texture_callback(&ui_texture_clbk);

  iom_init();
  ui_resize_window(1280, 720);

  struct rect_T rect = {.x = 0, .y = 0, .w = 400, .h = 400};
  ren_set_viewport(rect);

  ren_init();
  create_targets();

  // iom_set_event_callback(sdl_callback);
  start_camera();

  s32 texture_id = create_texture(TEXTURES_SOURCE_DIR "gato-joel.png");

  s32 p_id =
      ren_create_program_from_files(SHADERS_SOURCE_DIR "vertex_texture.glsl",
                                    SHADERS_SOURCE_DIR "fragment_texture.glsl");

  s32 cef_texture_id = create_ui_texture();

  struct rect_T rect_cef = {.x = 0, .y = 0, .w = 1280, .h = 720};
  ren_set_ui_background(cef_texture_id, rect_cef);

  s32 e1 = ren_create_entity();
  s32 cube = ren_primitive_create_cube();
  s32 m_id = ren_create_material();
  ren_material_set_texture(m_id, texture_id);

  ren_entity_add_component(e1, COMPONENT_MATERIAL, m_id);
  ren_entity_add_component(e1, COMPONENT_OBJECT, cube);
  ren_entity_add_component(e1, COMPONENT_PROGRAM, p_id);

  while (!iom_can_close()) {
    iom_poll_events();
    ren_draw_frame();
    ui_message_loop();
  }

  ui_close();
  iom_quit();

  return 0;
};
