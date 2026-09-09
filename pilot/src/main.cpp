#include "SDL3/SDL_events.h"
#include "SDL3/SDL_mouse.h"

#include "ui.h"
#include "ui_msg_browser.h"
#include "ui_msg_common.h"

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

s32 pilot_create_texture(const char *file) {
  s32 width, height, nr_channels;
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

c16 pilot_sdl_key_to_c16(SDL_Keycode key, SDL_Keymod mod) {
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

void pilot_send_push_message();

void pilot_ipc_test_fnc(msg_T msg, msg_T response) {
  s32 num;
  ui_msg_arg_read_s32(msg, &num);
  INFO("Number is not this no?: %d", num);

  s32 result = 999;
  ui_msg_populate(response,
                  "holaque tal estan"); // ui_msg_write_s32(response, result);

  // pilot_send_push_message();
  //  ui_ipc_stream_write_arg(&response->it, (void *)&result, S32);
};

void pilot_set_ipc_calls() {
  enum ARG_TYPE i[1] = {S32};
  struct args args_i = (struct args){.args = i, .n_args = 1};
  enum ARG_TYPE o[1] = {STRING};
  struct args args_o = (struct args){.args = o, .n_args = 1};

  ui_msg_pull_new_entry("another_call", pilot_ipc_test_fnc, &args_i, &args_o);
  ui_msg_pull_new_entry("onecall", pilot_ipc_test_fnc, &args_i, &args_o);

  INFO("SUCCESFULL ENTRY ADDED");

  ui_msg_push_new_entry("hello", &args_i);
}

void pilot_send_push_message() {
  msg_T msg = ui_msg_push_create("hello");
  if (!msg) {
    return;
  }
  // ui_msg_write_s32(msg, 2);
  ui_msg_populate(msg, 2);
  ui_msg_push_send(msg);
}

void pilot_sdl_gl_callback(SDL_Event *e) {}

static constexpr u32 PILOT_CEF_LEFT_MOD = 1u << 4;
static constexpr u32 PILOT_CEF_MIDDLE_MOD = 1u << 5;
static constexpr u32 PILOT_CEF_RIGHT_MOD = 1u << 6;

static u32 pilot_sdl_held_mods_ = 0;

static u32 pilot_sdl_button_to_mod(u8 button) {
  switch (button) {
  case SDL_BUTTON_LEFT:
    return PILOT_CEF_LEFT_MOD;
  case SDL_BUTTON_MIDDLE:
    return PILOT_CEF_MIDDLE_MOD;
  case SDL_BUTTON_RIGHT:
    return PILOT_CEF_RIGHT_MOD;
  default:
    return 0;
  }
}

static enum MOUSE_BTN pilot_sdl_button_to_mbtn(u8 button) {
  switch (button) {
  case SDL_BUTTON_MIDDLE:
    return MBTN_MIDDLE;
  case SDL_BUTTON_RIGHT:
    return MBTN_RIGHT;
  default:
    return MBTN_LEFT;
  }
}

void pilot_sdl_ui_callback(SDL_Event *e) {

  switch (e->type) {
  case SDL_EVENT_KEY_DOWN: {
    char16_t ch = pilot_sdl_key_to_c16(e->key.key, SDL_GetModState());
    ui_send_mouse_keydown(ch);
    break;
  }
  case SDL_EVENT_KEY_UP: {

    char16_t ch = pilot_sdl_key_to_c16(e->key.key, SDL_GetModState());
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
    ui_send_mouse_event_motion(m_p, pilot_sdl_held_mods_);
    break;
  }

  case SDL_EVENT_MOUSE_BUTTON_DOWN: {
    u32 mod = pilot_sdl_button_to_mod(e->button.button);
    pilot_sdl_held_mods_ |= mod;
    point_T m_p = {.x = (s32)e->button.x, .y = (s32)e->button.y};
    ui_send_mouse_down(pilot_sdl_button_to_mbtn(e->button.button), m_p,
                       pilot_sdl_held_mods_);
    ui_send_mouse_event_motion(m_p, pilot_sdl_held_mods_);
    break;
  }

  case SDL_EVENT_MOUSE_BUTTON_UP: {
    u32 mod = pilot_sdl_button_to_mod(e->button.button);
    point_T m_p = {.x = (s32)e->button.x, .y = (s32)e->button.y};
    ui_send_mouse_up(pilot_sdl_button_to_mbtn(e->button.button), m_p, pilot_sdl_held_mods_);
    pilot_sdl_held_mods_ &= ~mod;
    ui_send_mouse_event_motion(m_p, pilot_sdl_held_mods_);
    break;
  }
  default:
    break;
  }
}

s32 pilot_create_ui_texture() {
  // TODO: temporary
  u8 *buffer = new u8[1280 * 720 * 4];
  for (s32 i = 0; i < 1280; i++) {
    for (s32 j = 0; j < 720; j++) {
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

void pilot_start_camera() {
  ren_set_camera_aspect_ratio_wh(800, 600);
  ren_set_camera_planes(0.1f, 100.0f);
  ren_set_camera_projection(PERSPECTIVE);
  ren_set_camera_fov(1.3);
  ren_camera_set_position(0.0, 0.0, 5.0);
  ren_camera_set_rotation(0.0, 0.0, 0.0);
}

void pilot_ui_texture_clbk(u8 *bitmap, u32 width, u32 height) {
  // TODO: think a clever way to do that. Temporal.
  // struct material_T mat = ren_get_material(g_hud_m_id);

  struct rect_T b = {
      .x = 0, .y = 0, .w = (s32)width, .h = (s32)height}; // Potential
                                                          // oberflow
                                                          // bug
  ren_update_ui_background(b, bitmap);
}

static s32 pilot_ui_cursor_to_iom(s32 ui_cursor) {
  switch (ui_cursor) {
  case UI_CURSOR_CROSS:
    return IOM_CURSOR_CROSSHAIR;
  case UI_CURSOR_HAND:
    return IOM_CURSOR_POINTER;
  case UI_CURSOR_IBEAM:
  case UI_CURSOR_VERTICALTEXT:
    return IOM_CURSOR_TEXT;
  case UI_CURSOR_WAIT:
    return IOM_CURSOR_WAIT;
  case UI_CURSOR_PROGRESS:
    return IOM_CURSOR_PROGRESS;
  case UI_CURSOR_MOVE:
  case UI_CURSOR_MIDDLEPANNING:
  case UI_CURSOR_EASTPANNING:
  case UI_CURSOR_NORTHPANNING:
  case UI_CURSOR_NORTHEASTPANNING:
  case UI_CURSOR_NORTHWESTPANNING:
  case UI_CURSOR_SOUTHPANNING:
  case UI_CURSOR_SOUTHEASTPANNING:
  case UI_CURSOR_SOUTHWESTPANNING:
  case UI_CURSOR_WESTPANNING:
  case UI_CURSOR_GRAB:
  case UI_CURSOR_GRABBING:
  case UI_CURSOR_DND_MOVE:
    return IOM_CURSOR_MOVE;
  case UI_CURSOR_CELL:
  case UI_CURSOR_ZOOMIN:
  case UI_CURSOR_ZOOMOUT:
    return IOM_CURSOR_CROSSHAIR;
  case UI_CURSOR_ALIAS:
  case UI_CURSOR_COPY:
  case UI_CURSOR_DND_COPY:
  case UI_CURSOR_DND_LINK:
    return IOM_CURSOR_POINTER;
  case UI_CURSOR_NODROP:
  case UI_CURSOR_NOTALLOWED:
  case UI_CURSOR_DND_NONE:
    return IOM_CURSOR_NOT_ALLOWED;
  case UI_CURSOR_EASTRESIZE:
  case UI_CURSOR_WESTRESIZE:
  case UI_CURSOR_EASTWESTRESIZE:
  case UI_CURSOR_COLUMNRESIZE:
    return IOM_CURSOR_EW_RESIZE;
  case UI_CURSOR_NORTHRESIZE:
  case UI_CURSOR_SOUTHRESIZE:
  case UI_CURSOR_NORTHSOUTHRESIZE:
  case UI_CURSOR_ROWRESIZE:
    return IOM_CURSOR_NS_RESIZE;
  case UI_CURSOR_NORTHEASTRESIZE:
  case UI_CURSOR_SOUTHWESTRESIZE:
  case UI_CURSOR_NORTHEASTSOUTHWESTRESIZE:
    return IOM_CURSOR_NESW_RESIZE;
  case UI_CURSOR_NORTHWESTRESIZE:
  case UI_CURSOR_SOUTHEASTRESIZE:
  case UI_CURSOR_NORTHWESTSOUTHEASTRESIZE:
    return IOM_CURSOR_NWSE_RESIZE;
  case UI_CURSOR_MIDDLE_PANNING_VERTICAL:
    return IOM_CURSOR_NS_RESIZE;
  case UI_CURSOR_MIDDLE_PANNING_HORIZONTAL:
    return IOM_CURSOR_EW_RESIZE;
  default:
    return IOM_CURSOR_DEFAULT;
  }
}

void pilot_ui_cursor_clbk(s32 cursor_type) {
  iom_set_cursor(pilot_ui_cursor_to_iom(cursor_type));
}

void pilot_create_targets() {

  g_gl_target = iom_create_target();
  g_ui_target = iom_create_target();

  struct rect_T b_ui = {.x = 0, .y = 0, .w = 1280, .h = 720}; // TODO: hardcoded
                                                              // !URGENT

  // !URGENT
  iom_set_target(g_ui_target, b_ui, 1, pilot_sdl_ui_callback);

  // SDL 0, 0 is top-left and GL and CEF are bottom-left.
  struct point_T w_s = iom_get_window_size();
  INFO("gl bounds: w - %d | h - %d", w_s.x, w_s.y);
  struct rect_T b_gl = {
      .x = 0, .y = w_s.y - 400, .w = 400, .h = 400}; // TODO: hardcoded
  iom_set_target(g_gl_target, b_gl, 2, pilot_sdl_gl_callback);
}

int main(int argc, char *argv[]) {
  // INFO("HOLAAA");

  if (ui_start(argc, argv) != 0) {
    return -1;
  }

  pilot_set_ipc_calls();
  ui_set_ui_texture_callback(&pilot_ui_texture_clbk);
  ui_set_cursor_callback(&pilot_ui_cursor_clbk);
  ui_resize_window(1280, 720);

  iom_init();
  pilot_create_targets();

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
  pilot_start_camera();

  s32 texture_id = pilot_create_texture(TEXTURES_SOURCE_DIR "gato-joel.png");

  s32 p_id =
      ren_create_program_from_files(SHADERS_SOURCE_DIR "vertex_texture.glsl",
                                    SHADERS_SOURCE_DIR "fragment_texture.glsl");

  s32 cef_texture_id = pilot_create_ui_texture();

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
