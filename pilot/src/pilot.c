#include "pilot.h"

#include "dmath.h"
#include "global.h"
#include "io_manager.h"
#include "log.h"
#include "ui.h"
#include "ui_msg_browser.h"
#include "ui_msg_common.h"

#include "data/queue.h"
#include "render.h"
#include "texture.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <math.h>

#include "camera.h"
#include "entity.h"
#include "material.h"
#include "object.h"
#include "shader.h"
#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static u32 pilot_sdl_held_mods_ = 0;

static u32 PILOT_CEF_LEFT_MOD = 1u << 4;
static u32 PILOT_CEF_MIDDLE_MOD = 1u << 5;
static u32 PILOT_CEF_RIGHT_MOD = 1u << 6;

#define PILOT_LOG_PENDING_CAP 64

static queue_T pilot_log_pending_ = NULL;

static const char *pilot_ui_level_str_(enum pilot_ui_log_level level);

static const char *pilot_ui_level_str_(enum pilot_ui_log_level level) {
  switch (level) {
  case PILOT_UI_TRACE:
    return "TRACE";
  case PILOT_UI_DEBUG:
    return "DEBUG";
  case PILOT_UI_INFO:
    return "INFO";
  case PILOT_UI_WARN:
    return "WARN";
  case PILOT_UI_ERROR:
    return "ERROR";
  default:
    return "INFO";
  }
};

void pilot_ui_log_flush(void) {
  if (!pilot_log_pending_ || gen_queue_empty(pilot_log_pending_))
    return;
  while (!gen_queue_empty(pilot_log_pending_)) {
    if (ui_msg_push_listeners("pilot-log") <= 0)
      return;
    char *line = (char *)gen_queue_peek(pilot_log_pending_);
    msg_T msg = ui_msg_push_create("pilot-log");
    if (!msg)
      return;
    ui_msg_populate(msg, (c8 *)line);
    if (ui_msg_push_send(msg) != 0) {
      ui_msg_free(msg);
      return;
    }
    ui_msg_free(msg);
    gen_queue_pop(pilot_log_pending_);
    free(line);
  }
}

void pilot_ui_log(enum pilot_ui_log_level level, const c8 *fmt, ...) {
  if (!fmt) {
    return;
  }
  if (!pilot_log_pending_) {
    pilot_log_pending_ = gen_queue_new();
  }
  if (!pilot_log_pending_) {
    return;
  }
  while (gen_queue_size(pilot_log_pending_) >= PILOT_LOG_PENDING_CAP) {
    char *old = (char *)gen_queue_pop(pilot_log_pending_);
    if (!old) {
      break;
    }
    WARN("pilot log queue full (%d), dropping oldest: %s",
         PILOT_LOG_PENDING_CAP, old);
    free(old);
  }
  char tmp[512];
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(tmp, sizeof(tmp), (const char *)fmt, ap);
  va_end(ap);
  const char *ls = pilot_ui_level_str_(level);
  size_t n = strlen(ls) + strlen(tmp) + 4;
  char *line = (char *)malloc(n);
  if (!line) {
    return;
  }
  snprintf(line, n, "[%s] %s", ls, tmp);
  if (gen_queue_push(pilot_log_pending_, line) != 0) {
    free(line);
  }
};

void pilot_test_levels_fnc(msg_T msg, msg_T response) {
  (void)msg;
  pilot_ui_log(PILOT_UI_TRACE, "trace test message");
  pilot_ui_log(PILOT_UI_DEBUG, "debug test message");
  pilot_ui_log(PILOT_UI_INFO, "info test message");
  pilot_ui_log(PILOT_UI_WARN, "warn test message");
  pilot_ui_log(PILOT_UI_ERROR, "error test message");
  (void)response; // no out-args: browser sends no response
};

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

static c16 pilot_sdl_key_to_c16(SDL_Keycode key, SDL_Keymod mod);

static c16 pilot_sdl_key_to_c16(SDL_Keycode key, SDL_Keymod mod) {
  bool shift = (mod & SDL_KMOD_SHIFT) != 0;

  if (key >= SDLK_A && key <= SDLK_Z) {
    return (c16)(shift ? (key - 'a' + 'A') : key);
  }

  if (key >= SDLK_0 && key <= SDLK_9) {
    if (!shift)
      return (c16)(key);

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

static void pilot_sdl_gl_callback(SDL_Event *e);

#define PILOT_ORBIT_ROT_SENS 0.005f
#define PILOT_ORBIT_PAN_SENS 0.002f

static s32 orbit_drag_btn_ = 0;
static bool orbit_drag_pan_ = false;

static void pilot_sdl_gl_callback(SDL_Event *e) {
  switch (e->type) {
  case SDL_EVENT_MOUSE_BUTTON_DOWN: {
    if (e->button.button != SDL_BUTTON_MIDDLE &&
        e->button.button != SDL_BUTTON_RIGHT) {
      break;
    }
    orbit_drag_btn_ = e->button.button;
    orbit_drag_pan_ = (e->button.button == SDL_BUTTON_RIGHT) ||
                      ((SDL_GetModState() & SDL_KMOD_SHIFT) != 0);
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (e->button.button == orbit_drag_btn_) {
      orbit_drag_btn_ = 0;
      orbit_drag_pan_ = false;
    }
    break;
  }
  case SDL_EVENT_MOUSE_MOTION: {
    if (orbit_drag_btn_ == 0) {
      break;
    }
    if (orbit_drag_pan_) {
      f32 dist = ren_camera_orbit_get_r();
      f32 yaw = ren_camera_orbit_get_yaw();
      f32 s = PILOT_ORBIT_PAN_SENS * dist;
      f32 rx = cosf(yaw);
      f32 rz = -sinf(yaw);
      vec3 delta = {-((f32)e->motion.xrel) * s * rx,
                    ((f32)e->motion.yrel) * s,
                    -((f32)e->motion.xrel) * s * rz};
      ren_translate_camera(delta);
    } else {
      vec3 delta = {-((f32)e->motion.yrel) * PILOT_ORBIT_ROT_SENS,
                    -((f32)e->motion.xrel) * PILOT_ORBIT_ROT_SENS, 0.0f};
      ren_camera_orbit(delta);
    }
    break;
  }
  case SDL_EVENT_MOUSE_WHEEL: {
    f32 r = ren_camera_orbit_get_r();
    if (e->wheel.y > 0.0f) {
      ren_camera_orbit_r(r * 0.9f);
    } else if (e->wheel.y < 0.0f) {
      ren_camera_orbit_r(r * (1.0f / 0.9f));
    }
    break;
  }
  default:
    break;
  }
};

static void pilot_sdl_ui_callback(SDL_Event *e);

static void pilot_sdl_ui_callback(SDL_Event *e) {

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
    g_cfg.win_w = width;
    g_cfg.win_h = height;
    g_cfg.ui_bounds.x = 0;
    g_cfg.ui_bounds.y = 0;
    g_cfg.ui_bounds.w = width;
    g_cfg.ui_bounds.h = height;
    ui_resize_window(width, height);

    struct rect_T b = {.x = 0, .y = 0, .w = width, .h = height};
    iom_resize_target(g_ui_target, b);

    break;
  }

  case SDL_EVENT_MOUSE_MOTION: {
    struct point_T m_p = {.x = (s32)e->motion.x, .y = (s32)e->motion.y};
    ui_send_mouse_event_motion(m_p, pilot_sdl_held_mods_);
    break;
  }

  case SDL_EVENT_MOUSE_BUTTON_DOWN: {
    u32 mod = pilot_sdl_button_to_mod(e->button.button);
    pilot_sdl_held_mods_ |= mod;
    struct point_T m_p = {.x = (s32)e->button.x, .y = (s32)e->button.y};
    ui_send_mouse_down(pilot_sdl_button_to_mbtn(e->button.button), m_p,
                       pilot_sdl_held_mods_);
    ui_send_mouse_event_motion(m_p, pilot_sdl_held_mods_);
    break;
  }

  case SDL_EVENT_MOUSE_BUTTON_UP: {
    u32 mod = pilot_sdl_button_to_mod(e->button.button);
    struct point_T m_p = {.x = (s32)e->button.x, .y = (s32)e->button.y};
    ui_send_mouse_up(pilot_sdl_button_to_mbtn(e->button.button), m_p,
                     pilot_sdl_held_mods_);
    pilot_sdl_held_mods_ &= ~mod;
    ui_send_mouse_event_motion(m_p, pilot_sdl_held_mods_);
    break;
  }
  default:
    break;
  }
}

static void pilot_viewport_clbk(msg_T msg, msg_T response);

void pilot_viewport_clbk(msg_T msg, msg_T response) {
  s32 x, y, w, h;
  ui_msg_arg_read_s32(msg, &x);
  ui_msg_arg_read_s32(msg, &y);
  ui_msg_arg_read_s32(msg, &w);
  ui_msg_arg_read_s32(msg, &h);
  if (w <= 0 || h <= 0) {
    WARN("gl-viewport: ignoring non-positive size %d x %d", w, h);
    return;
  }
  struct point_T ws = iom_get_window_size();
  if (w > ws.x)
    w = ws.x;
  if (h > ws.y)
    h = ws.y;
  iom_resize_target(g_gl_target,
                    (struct rect_T){.x = x, .y = y, .w = w, .h = h});
  s32 y_gl = ws.y - (y + h);
  if (y_gl < 0)
    y_gl = 0;
  ren_set_viewport((struct rect_T){.x = x, .y = y_gl, .w = w, .h = h});
  s32 first = !g_cfg.gl_ready;
  g_cfg.gl_bounds.x = x;
  g_cfg.gl_bounds.y = y_gl;
  g_cfg.gl_bounds.w = w;
  g_cfg.gl_bounds.h = h;
  g_cfg.gl_ready = 1;
  if (first) {
    INFO("gl target ready: %d,%d %d x %d", x, y_gl, w, h);
    return;
  }
  INFO("gl-viewport: %d,%d %d x %d", x, y, w, h);
  (void)response;
};
void pilot_set_msg_calls() {
  ui_msg_pull_new("gl-viewport", pilot_viewport_clbk);
  ui_msg_pull_set_i("gl-viewport", S32, S32, S32, S32);
  ui_msg_pull_set_o("gl-viewport");

  enum ARG_TYPE log_o[1] = {STRING};
  struct args log_args_o = (struct args){.args = log_o, .n_args = 1};
  ui_msg_push_new_entry("pilot-log", &log_args_o);
  // ui_msg_push_set_i();
  // ui_msg_push_set_o();
}

void pilot_ui_texture_clbk(u8 *bitmap, u32 width, u32 height) {
  struct rect_T b = {.x = 0, .y = 0, .w = (s32)width, .h = (s32)height};
  if (g_cfg.cef_texture == 0) {
    s32 id = ren_create_texture(bitmap, (s32)width, (s32)height, 4, BGRA);
    if (id <= 0) {
      ERROR("cef texture create failed");
      return;
    }
    ren_set_ui_background(id, b);
    g_cfg.cef_texture = id;
    g_cfg.ui_bounds = b;
    return;
  }
  g_cfg.ui_bounds = b;
  ren_update_ui_background(b, bitmap);
}

static void pilot_start_camera();

static void pilot_start_camera() {
  res_init_camera();
  ren_set_camera_aspect_ratio_wh(800, 600);
  ren_set_camera_planes(0.1f, 100.0f);
  ren_set_camera_projection(PERSPECTIVE);
  ren_set_camera_fov(1.3);
  ren_camera_set_position(0.0, 0.0, 5.0);
  ren_camera_set_rotation(0.0, 0.0, 0.0);
  ren_translate_camera((vec3){0.0f, 0.0f, -3.0f});
  ren_camera_mode(CAMERA_ORBIT);
}

void pilot_init_scene() {

  pilot_start_camera();

  s32 texture_id = pilot_create_texture(TEXTURES_SOURCE_DIR "gato-joel.png");

  s32 p_id =
      ren_create_program_from_files(SHADERS_SOURCE_DIR "vertex_texture.glsl",
                                    SHADERS_SOURCE_DIR "fragment_texture.glsl");

  s32 e1 = ren_create_entity();
  s32 cube = ren_primitive_create_cube();
  s32 m_id = ren_create_material();
  ren_material_set_texture(m_id, texture_id);

  ren_entity_add_component(e1, COMPONENT_MATERIAL, m_id);
  ren_entity_add_component(e1, COMPONENT_OBJECT, cube);
  ren_entity_add_component(e1, COMPONENT_PROGRAM, p_id);
};

void pilot_create_targets() {

  g_gl_target = iom_create_target();
  g_ui_target = iom_create_target();

  struct rect_T b_ui = g_cfg.ui_bounds;
  // !URGENT
  iom_set_target(g_ui_target, b_ui, 1, pilot_sdl_ui_callback);
  // The UI keeps receiving every event (typing survives mouse drift);
  // the GL panel does not track.
  iom_target_set_flag(g_ui_target, TARGET_CALLBACK_ALWAYS);

  // GL target stays zero-sized until the first gl-viewport message arrives.
  struct rect_T b_gl = {.x = 0, .y = 0, .w = 0, .h = 0};
  iom_set_target(g_gl_target, b_gl, 2, pilot_sdl_gl_callback);
}
