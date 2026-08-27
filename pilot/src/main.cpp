#include "SDL3/SDL_events.h"
#include "SDL3/SDL_mouse.h"

#include "ui.h"
#include "ui_ipc.h"
#include <cstring>

extern "C" {
#include "camera.h"
#include "dmath.h"
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

c16 sdl_key_to_c16(SDL_Keycode key, SDL_Keymod mod) {
  bool shift = (mod & SDL_KMOD_SHIFT) != 0;

  if (key >= SDLK_A && key <= SDLK_Z) {
    return static_cast<char16_t>(shift ? (key - 'a' + 'A') : key);
  }

  if (key >= SDLK_0 && key <= SDLK_9) {
    if (!shift)
      return static_cast<char16_t>(key);

    switch (key) {
    case SDLK_1:
      return u'!';
    case SDLK_2:
      return u'@';
    case SDLK_3:
      return u'#';
    case SDLK_4:
      return u'$';
    case SDLK_5:
      return u'%';
    case SDLK_6:
      return u'^';
    case SDLK_7:
      return u'&';
    case SDLK_8:
      return u'*';
    case SDLK_9:
      return u'(';
    case SDLK_0:
      return u')';
    }
  }

  switch (key) {
  case SDLK_RETURN:
    return u'\r';
  case SDLK_BACKSPACE:
    return u'\b';
  case SDLK_TAB:
    return u'\t';
  case SDLK_SPACE:
    return u' ';
  case SDLK_ESCAPE:
    return 0x1B;

  case SDLK_MINUS:
    return shift ? u'_' : u'-';
  case SDLK_EQUALS:
    return shift ? u'+' : u'=';
  case SDLK_LEFTBRACKET:
    return shift ? u'{' : u'[';
  case SDLK_RIGHTBRACKET:
    return shift ? u'}' : u']';
  case SDLK_BACKSLASH:
    return shift ? u'|' : u'\\';
  case SDLK_SEMICOLON:
    return shift ? u':' : u';';
  case SDLK_APOSTROPHE:
    return shift ? u'"' : u'\'';
  case SDLK_GRAVE:
    return shift ? u'~' : u'`';
  case SDLK_COMMA:
    return shift ? u'<' : u',';
  case SDLK_PERIOD:
    return shift ? u'>' : u'.';
  case SDLK_SLASH:
    return shift ? u'?' : u'/';

  default:
    return 0;
  }
}

void ipc_test_fnc(msg_T msg, msg_T response) {
  s32 num;
  ui_msg_arg_read_s32(msg, &num);
  INFO("Number: %d", num);
  INFO("key clbk: %s", ui_msg_bitstream(msg));

  int result = 23;
  ui_msg_push_s32(response, result);
  // ui_ipc_stream_write_arg(&response->it, (void *)&result, S32);
};

void set_ipc_calls() {
  enum ARG_TYPE i[1] = {S32};
  struct args args_i = (struct args){.args = i, .n_args = 1};
  enum ARG_TYPE o[1] = {S32};
  struct args args_o = (struct args){.args = o, .n_args = 1};

  ui_msg_pull_new_entry("another_call", ipc_test_fnc, &args_i, &args_o);
  ui_msg_pull_new_entry("onecall", ipc_test_fnc, &args_i, &args_o);

  INFO("SUCCESFULL ENTRY ADDED");
}

void sdl_gl_callback(SDL_Event *e) {}

void sdl_ui_callback(SDL_Event *e) {

  switch (e->type) {
  case SDL_EVENT_KEY_DOWN: {
    char16_t ch = sdl_key_to_c16(e->key.key, SDL_GetModState());
    ui_send_mouse_keydown(ch);
    break;
  }
  case SDL_EVENT_KEY_UP: {

    char16_t ch = sdl_key_to_c16(e->key.key, SDL_GetModState());
    ui_send_mouse_keyup(ch);
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

  case SDL_EVENT_MOUSE_MOTION: {
    point_T m_p = {.x = (s32)e->motion.x, .y = (s32)e->motion.y};
    ui_send_mouse_event_motion(m_p);
    break;
  }

  case SDL_EVENT_MOUSE_BUTTON_DOWN: {
    switch (e->button.button) {
    case SDL_BUTTON_LEFT: {
      point_T m_p = {.x = (s32)e->motion.x, .y = (s32)e->motion.y};
      ui_send_mouse_event_click(MBTN_LEFT, m_p);
    }
    default:
      break;
    }
    point_T m_p = {.x = (s32)e->motion.x, .y = (s32)e->motion.y};
    ui_send_mouse_event_motion(m_p);
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
  // TODO: think a clever way to do that. Temporal.
  // struct material_T mat = ren_get_material(g_hud_m_id);

  struct rect_T b = {
      .x = 0, .y = 0, .w = (s32)width, .h = (s32)height}; // Potential
                                                          // oberflow
                                                          // bug
  ren_update_ui_background(b, bitmap);
}

void create_targets() {

  g_gl_target = iom_create_target();
  g_ui_target = iom_create_target();

  struct rect_T b_ui = {.x = 0, .y = 0, .w = 1280, .h = 720}; // TODO: hardcoded
                                                              // !URGENT

  // !URGENT
  iom_set_target(g_ui_target, b_ui, 1, sdl_ui_callback);

  // SDL 0, 0 is top-left and GL and CEF are bottom-left.
  struct point_T w_s = iom_get_window_size();
  INFO("gl bounds: w - %d | h - %d", w_s.x, w_s.y);
  struct rect_T b_gl = {
      .x = 0, .y = w_s.y - 400, .w = 400, .h = 400}; // TODO: hardcoded
  iom_set_target(g_gl_target, b_gl, 2, sdl_gl_callback);
}

int main(int argc, char *argv[]) {

  if (ui_start(argc, argv) != 0) {
    return -1;
  }

  set_ipc_calls();
  ui_set_ui_texture_callback(&ui_texture_clbk);
  ui_resize_window(1280, 720);

  iom_init();
  create_targets();

  // start_ui
  //
  // signal UI started correctly
  // initialize messages
  //
  // then start iom
  //
  // start rm
  //
  // main loop
  //
  // free

  struct rect_T rect = {.x = 0, .y = 0, .w = 400, .h = 400};
  ren_set_viewport(rect);
  ren_init();

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
