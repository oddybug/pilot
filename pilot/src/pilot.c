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
  (void)response;
};

static void pilot_json_escape_(const char *src, char *dst, size_t cap);

static void pilot_get_program_clbk(msg_T msg, msg_T response) {
  s32 id;
  ui_msg_arg_read_s32(msg, &id);
  if (id <= 0 || id >= MAX_PROGRAMS || !programs[id].id) {
    ui_msg_populate(response, (c8 *)"{}");
    return;
  }
  const c8 *n = programs[id].name;
  char esc[128];
  char out[1024];
  if (n)
    pilot_json_escape_(n, esc, sizeof(esc));
  else
    esc[0] = '\0';
  if (n && n[0])
    snprintf(
        out, sizeof(out),
        "{\"id\":%d,\"name\":\"%s\",\"gl_id\":%u,\"vs_id\":%u,\"fs_id\":%u}",
        id, esc, programs[id].id, programs[id].vs_id, programs[id].fs_id);
  else
    snprintf(out, sizeof(out),
             "{\"id\":%d,\"gl_id\":%u,\"vs_id\":%u,\"fs_id\":%u}", id,
             programs[id].id, programs[id].vs_id, programs[id].fs_id);
  ui_msg_populate(response, (c8 *)out);
}

static void pilot_get_object_clbk(msg_T msg, msg_T response) {
  s32 id;
  ui_msg_arg_read_s32(msg, &id);
  if (id <= 0 || id >= MAX_OBJECTS ||
      (!objects[id].VAO && !objects[id].n_triangles)) {
    ui_msg_populate(response, (c8 *)"{}");
    return;
  }
  const c8 *n = objects[id].name;
  char esc[128];
  char out[1024];
  if (n)
    pilot_json_escape_(n, esc, sizeof(esc));
  else
    esc[0] = '\0';
  if (n && n[0])
    snprintf(
        out, sizeof(out),
        "{\"id\":%d,\"name\":\"%s\",\"vao\":%u,\"n_tri\":%u,\"vbo\":[%u,%u,%u],"
        "\"pos\":[%.2f,%.2f,%.2f],\"rot\":[%.2f,%.2f,%.2f]}",
        id, esc, objects[id].VAO, objects[id].n_triangles, objects[id].VBO[0],
        objects[id].VBO[1], objects[id].VBO[2], objects[id].position[0],
        objects[id].position[1], objects[id].position[2],
        objects[id].rotation[0], objects[id].rotation[1],
        objects[id].rotation[2]);
  else
    snprintf(out, sizeof(out),
             "{\"id\":%d,\"vao\":%u,\"n_tri\":%u,\"vbo\":[%u,%u,%u],"
             "\"pos\":[%.2f,%.2f,%.2f],\"rot\":[%.2f,%.2f,%.2f]}",
             id, objects[id].VAO, objects[id].n_triangles, objects[id].VBO[0],
             objects[id].VBO[1], objects[id].VBO[2], objects[id].position[0],
             objects[id].position[1], objects[id].position[2],
             objects[id].rotation[0], objects[id].rotation[1],
             objects[id].rotation[2]);
  ui_msg_populate(response, (c8 *)out);
}

static void pilot_get_texture_clbk(msg_T msg, msg_T response) {
  s32 id;
  ui_msg_arg_read_s32(msg, &id);
  if (id <= 0 || id >= MAX_TEXTURES || !textures[id].gl_id) {
    ui_msg_populate(response, (c8 *)"{}");
    return;
  }
  const c8 *n = textures[id].name;
  char esc[128];
  char out[1024];
  if (n)
    pilot_json_escape_(n, esc, sizeof(esc));
  else
    esc[0] = '\0';
  const char *ts = textures[id].type == RGB    ? "RGB"
                   : textures[id].type == RGBA ? "RGBA"
                   : textures[id].type == BGRA ? "BGRA"
                                               : "UNKNOWN";
  if (n && n[0])
    snprintf(out, sizeof(out),
             "{\"id\":%d,\"name\":\"%s\",\"gl_id\":%u,\"w\":%d,\"h\":%d,"
             "\"type\":\"%s\"}",
             id, esc, textures[id].gl_id, textures[id].width,
             textures[id].height, ts);
  else
    snprintf(out, sizeof(out),
             "{\"id\":%d,\"gl_id\":%u,\"w\":%d,\"h\":%d,\"type\":\"%s\"}", id,
             textures[id].gl_id, textures[id].width, textures[id].height, ts);
  ui_msg_populate(response, (c8 *)out);
}

static void pilot_get_material_clbk(msg_T msg, msg_T response) {
  s32 id;
  ui_msg_arg_read_s32(msg, &id);
  if (id <= 0 || id >= MAX_MATERIALS ||
      (!materials[id].texture && !materials[id].program)) {
    ui_msg_populate(response, (c8 *)"{}");
    return;
  }
  const c8 *n = materials[id].name;
  char esc[128];
  char out[1024];
  if (n)
    pilot_json_escape_(n, esc, sizeof(esc));
  else
    esc[0] = '\0';
  s32 tid = materials[id].texture;
  s32 pid = materials[id].program;
  const c8 *tn = (tid > 0 && tid < MAX_TEXTURES) ? textures[tid].name : NULL;
  const c8 *pn = (pid > 0 && pid < MAX_PROGRAMS) ? programs[pid].name : NULL;
  char tesc[128], pesc[128];
  if (tn)
    pilot_json_escape_(tn, tesc, sizeof(tesc));
  else
    tesc[0] = '\0';
  if (pn)
    pilot_json_escape_(pn, pesc, sizeof(pesc));
  else
    pesc[0] = '\0';
  if (n && n[0])
    snprintf(out, sizeof(out),
             "{\"id\":%d,\"name\":\"%s\",\"program\":%d,\"programName\":\"%s\","
             "\"texture\":%d,\"textureName\":\"%s\"}",
             id, esc, pid, pesc, tid, tesc);
  else
    snprintf(out, sizeof(out),
             "{\"id\":%d,\"program\":%d,\"programName\":\"%s\",\"texture\":%d,"
             "\"textureName\":\"%s\"}",
             id, pid, pesc, tid, tesc);
  ui_msg_populate(response, (c8 *)out);
}

static void pilot_get_entity_clbk(msg_T msg, msg_T response) {
  s32 id;
  ui_msg_arg_read_s32(msg, &id);
  if (id <= 0 || id >= MAX_ENTITIES || !entities[id].id) {
    ui_msg_populate(response, (c8 *)"{}");
    return;
  }
  const c8 *n = entities[id].name;
  char esc[128];
  char out[1024];
  if (n)
    pilot_json_escape_(n, esc, sizeof(esc));
  else
    esc[0] = '\0';
  s32 m = entities[id].components[COMPONENT_MATERIAL];
  s32 o = entities[id].components[COMPONENT_OBJECT];
  s32 p = (m > 0 && m < MAX_MATERIALS) ? materials[m].program : 0;
  const c8 *pn = (p > 0 && p < MAX_PROGRAMS) ? programs[p].name : NULL;
  const c8 *mn = (m > 0 && m < MAX_MATERIALS) ? materials[m].name : NULL;
  const c8 *on = (o > 0 && o < MAX_OBJECTS) ? objects[o].name : NULL;
  char pesc[128], mesc[128], oesc[128];
  if (pn)
    pilot_json_escape_(pn, pesc, sizeof(pesc));
  else
    pesc[0] = '\0';
  if (mn)
    pilot_json_escape_(mn, mesc, sizeof(mesc));
  else
    mesc[0] = '\0';
  if (on)
    pilot_json_escape_(on, oesc, sizeof(oesc));
  else
    oesc[0] = '\0';
  if (n && n[0])
    snprintf(out, sizeof(out),
             "{\"id\":%d,\"name\":\"%s\",\"program\":%d,\"programName\":\"%s\","
             "\"material\":%d,\"materialName\":\"%s\",\"object\":%d,"
             "\"objectName\":\"%s\"}",
             id, esc, p, pesc, m, mesc, o, oesc);
  else
    snprintf(out, sizeof(out),
             "{\"id\":%d,\"program\":%d,\"programName\":\"%s\",\"material\":%d,"
             "\"materialName\":\"%s\",\"object\":%d,\"objectName\":\"%s\"}",
             id, p, pesc, m, mesc, o, oesc);
  ui_msg_populate(response, (c8 *)out);
};

static void pilot_set_object_transform_clbk(msg_T msg, msg_T response) {
  s32 id;
  f32 v[6];
  ui_msg_arg_read_s32(msg, &id);
  for (int i = 0; i < 6; i++)
    ui_msg_arg_read_f32(msg, &v[i]);
  if (id <= 0 || id >= MAX_OBJECTS ||
      (!objects[id].VAO && !objects[id].n_triangles)) {
    WARN("set-object-transform: bad object %d", id);
    return;
  }
  objects[id].position[0] = v[0];
  objects[id].position[1] = v[1];
  objects[id].position[2] = v[2];
  objects[id].rotation[0] = v[3];
  objects[id].rotation[1] = v[4];
  objects[id].rotation[2] = v[5];
  (void)response;
};

static void pilot_set_entity_material_clbk(msg_T msg, msg_T response) {
  s32 e_id, m_id;
  ui_msg_arg_read_s32(msg, &e_id);
  ui_msg_arg_read_s32(msg, &m_id);
  if (e_id <= 0 || e_id >= MAX_ENTITIES || !entities[e_id].id) {
    WARN("set-entity-material: bad entity %d", e_id);
    return;
  }
  if (m_id <= 0 || m_id >= MAX_MATERIALS ||
      (!materials[m_id].program && !materials[m_id].texture)) {
    WARN("set-entity-material: bad material %d", m_id);
    return;
  }
  ren_entity_add_component((u32)e_id, COMPONENT_MATERIAL, (u32)m_id);
  (void)response;
};

static void pilot_json_escape_(const char *src, char *dst, size_t cap) {
  size_t o = 0;
  for (size_t i = 0; src[i] && o + 2 < cap; i++) {
    if (src[i] == '"' || src[i] == '\\') {
      dst[o++] = '\\';
      dst[o++] = src[i];
    } else if ((unsigned char)src[i] < 0x20) {
      dst[o++] = ' ';
    } else {
      dst[o++] = src[i];
    }
  }
  dst[o] = '\0';
}

static void pilot_render_list_json_(char *buf, size_t cap, s32 *ids,
                                    const c8 **names, u32 n) {
  size_t off = 0;
  off += snprintf(buf + off, cap - off, "[");
  for (u32 i = 0; i < n; i++) {
    char esc[128];
    if (names[i])
      pilot_json_escape_(names[i], esc, sizeof(esc));
    else
      esc[0] = '\0';
    if (names[i])
      off +=
          snprintf(buf + off, cap > off ? cap - off : 0,
                   "%s{\"id\":%d,\"name\":\"%s\"}", i ? "," : "", ids[i], esc);
    else
      off += snprintf(buf + off, cap > off ? cap - off : 0, "%s{\"id\":%d}",
                      i ? "," : "", ids[i]);
  }
  snprintf(buf + off, cap > off ? cap - off : 0, "]");
}

static void pilot_list_programs_clbk(msg_T msg, msg_T response) {
  (void)msg;
  s32 ids[MAX_PROGRAMS];
  const c8 *names[MAX_PROGRAMS];
  u32 n = 0;
  for (s32 i = 1; i < MAX_PROGRAMS; i++)
    if (programs[i].id) {
      ids[n] = i;
      names[n] = programs[i].name;
      n++;
    }
  char json[24576];
  pilot_render_list_json_(json, sizeof(json), ids, names, n);
  ui_msg_populate(response, (c8 *)json);
}

static void pilot_list_objects_clbk(msg_T msg, msg_T response) {
  (void)msg;
  s32 ids[MAX_OBJECTS];
  const c8 *names[MAX_OBJECTS];
  u32 n = 0;
  for (s32 i = 1; i < MAX_OBJECTS; i++)
    if (objects[i].VAO || objects[i].n_triangles) {
      ids[n] = i;
      names[n] = objects[i].name;
      n++;
    }
  char json[24576];
  pilot_render_list_json_(json, sizeof(json), ids, names, n);
  ui_msg_populate(response, (c8 *)json);
}

static void pilot_list_textures_clbk(msg_T msg, msg_T response) {
  (void)msg;
  s32 ids[MAX_TEXTURES];
  const c8 *names[MAX_TEXTURES];
  u32 n = 0;
  for (s32 i = 1; i < MAX_TEXTURES; i++)
    if (textures[i].gl_id) {
      ids[n] = i;
      names[n] = textures[i].name;
      n++;
    }
  char json[24576];
  pilot_render_list_json_(json, sizeof(json), ids, names, n);
  ui_msg_populate(response, (c8 *)json);
}

static void pilot_list_materials_clbk(msg_T msg, msg_T response) {
  (void)msg;
  s32 ids[MAX_MATERIALS];
  const c8 *names[MAX_MATERIALS];
  u32 n = 0;
  for (s32 i = 1; i < MAX_MATERIALS; i++)
    if (materials[i].texture || materials[i].program ||
        materials[i].has_color) {
      ids[n] = i;
      names[n] = materials[i].name;
      n++;
    }
  char json[24576];
  pilot_render_list_json_(json, sizeof(json), ids, names, n);
  ui_msg_populate(response, (c8 *)json);
}

static void pilot_list_entities_clbk(msg_T msg, msg_T response) {
  (void)msg;
  s32 ids[MAX_ENTITIES];
  const c8 *names[MAX_ENTITIES];
  u32 n = 0;
  for (s32 i = 1; i < MAX_ENTITIES; i++)
    if (entities[i].id) {
      ids[n] = i;
      names[n] = entities[i].name;
      n++;
    }
  char json[24576];
  pilot_render_list_json_(json, sizeof(json), ids, names, n);
  ui_msg_populate(response, (c8 *)json);
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

s32 pilot_create_texture(const c8 *name, const char *file) {
  s32 width, height, nr_channels;
  u8 *data = stbi_load(file, &width, &height, &nr_channels, 0);
  if (data) {

    s32 id = ren_create_texture(name, data, width, height, nr_channels, RGBA);
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
      vec3 delta = {-((f32)e->motion.xrel) * s * rx, ((f32)e->motion.yrel) * s,
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
  ren_set_camera_aspect_ratio_wh((f32)w, (f32)h);
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

#define PILOT_GL_Z_TOP 2
#define PILOT_GL_Z_LOW 0

static void pilot_viewport_enabled_clbk(msg_T msg, msg_T response);

static void pilot_viewport_enabled_clbk(msg_T msg, msg_T response) {
  s32 v;
  ui_msg_arg_read_s32(msg, &v);
  v = v != 0;
  g_cfg.gl_enabled = v;
  ren_set_gl_enabled(v);
  iom_target_z(g_gl_target, v ? PILOT_GL_Z_TOP : PILOT_GL_Z_LOW);
  INFO("gl-viewport-enabled: %d", v);
  (void)response;
};

static void pilot_skybox_clbk(msg_T msg, msg_T response);

static void pilot_skybox_clbk(msg_T msg, msg_T response) {
  u32 top;
  u32 bottom;
  s32 stiffness;
  ui_msg_arg_read_u32(msg, &top);
  ui_msg_arg_read_u32(msg, &bottom);
  ui_msg_arg_read_s32(msg, &stiffness);
  ren_skybox_set(top, bottom, stiffness);
  INFO("skybox-set: top 0x%06X bottom 0x%06X stiffness %d", top, bottom,
       stiffness);
  (void)response;
};

void pilot_set_msg_calls() {
  ui_msg_pull_new("gl-viewport", pilot_viewport_clbk);
  ui_msg_pull_set_i("gl-viewport", S32, S32, S32, S32);
  ui_msg_pull_set_o("gl-viewport");

  ui_msg_pull_new("gl-viewport-enabled", pilot_viewport_enabled_clbk);
  ui_msg_pull_set_i("gl-viewport-enabled", S32);
  ui_msg_pull_set_o("gl-viewport-enabled");

  ui_msg_pull_new("skybox-set", pilot_skybox_clbk);
  ui_msg_pull_set_i("skybox-set", U32, U32, S32);
  ui_msg_pull_set_o("skybox-set");

  ui_msg_pull_new("render-list-programs", pilot_list_programs_clbk);
  ui_msg_pull_set_i("render-list-programs");
  ui_msg_pull_set_o("render-list-programs", STRING);

  ui_msg_pull_new("render-list-objects", pilot_list_objects_clbk);
  ui_msg_pull_set_i("render-list-objects");
  ui_msg_pull_set_o("render-list-objects", STRING);

  ui_msg_pull_new("render-list-textures", pilot_list_textures_clbk);
  ui_msg_pull_set_i("render-list-textures");
  ui_msg_pull_set_o("render-list-textures", STRING);

  ui_msg_pull_new("render-list-materials", pilot_list_materials_clbk);
  ui_msg_pull_set_i("render-list-materials");
  ui_msg_pull_set_o("render-list-materials", STRING);

  ui_msg_pull_new("render-list-entities", pilot_list_entities_clbk);
  ui_msg_pull_set_i("render-list-entities");
  ui_msg_pull_set_o("render-list-entities", STRING);

  ui_msg_pull_new("render-get-program", pilot_get_program_clbk);
  ui_msg_pull_set_i("render-get-program", S32);
  ui_msg_pull_set_o("render-get-program", STRING);

  ui_msg_pull_new("render-get-object", pilot_get_object_clbk);
  ui_msg_pull_set_i("render-get-object", S32);
  ui_msg_pull_set_o("render-get-object", STRING);

  ui_msg_pull_new("render-get-texture", pilot_get_texture_clbk);
  ui_msg_pull_set_i("render-get-texture", S32);
  ui_msg_pull_set_o("render-get-texture", STRING);

  ui_msg_pull_new("render-get-material", pilot_get_material_clbk);
  ui_msg_pull_set_i("render-get-material", S32);
  ui_msg_pull_set_o("render-get-material", STRING);

  ui_msg_pull_new("render-get-entity", pilot_get_entity_clbk);
  ui_msg_pull_set_i("render-get-entity", S32);
  ui_msg_pull_set_o("render-get-entity", STRING);

  ui_msg_pull_new("set-entity-material", pilot_set_entity_material_clbk);
  ui_msg_pull_set_i("set-entity-material", S32, S32);
  ui_msg_pull_set_o("set-entity-material");

  ui_msg_pull_new("set-object-transform", pilot_set_object_transform_clbk);
  ui_msg_pull_set_i("set-object-transform", S32, F32, F32, F32, F32, F32, F32);
  ui_msg_pull_set_o("set-object-transform");

  enum ARG_TYPE log_o[1] = {STRING};
  struct args log_args_o = (struct args){.args = log_o, .n_args = 1};
  ui_msg_push_new_entry("pilot-log", &log_args_o);
  // ui_msg_push_set_i();
  // ui_msg_push_set_o();
}

void pilot_ui_texture_clbk(u8 *bitmap, s32 isSubImage, struct rect_T rect) {
  if (!g_cfg.cef_texture && !isSubImage) {
    s32 id = ren_create_texture(NULL, bitmap, rect.w, rect.h, 4, BGRA);
    if (id <= 0) {
      ERROR("cef texture create failed");
      return;
    }
    ren_set_ui_background(id, rect);
    g_cfg.cef_texture = id;
    return;
  }

  if (!isSubImage) {
    ren_update_ui_background(rect, bitmap);
  } else {
    if (g_cfg.cef_texture)
      ren_update_ui_background_sub(rect, bitmap);
  }
}

static void pilot_start_camera();

static void pilot_start_camera() {
  res_init_camera();
  ren_set_camera_aspect_ratio_wh(800, 600);
  ren_set_camera_planes(0.1f, 100.0f);
  ren_set_camera_projection(PERSPECTIVE);
  ren_set_camera_fov(1.3);
  // ren_translate_camera((vec3){0.0f, 0.0f, -10.0f});
  ren_camera_mode(CAMERA_ORBIT);
  ren_camera_orbit_o((vec3){0.0, 0.0, 0.0});
  ren_camera_orbit_r(6.0);
  ren_camera_orbit((vec3){-0.6, 1.0, 0.0});
}

void pilot_init_scene() {

  pilot_start_camera();

  s32 texture_id =
      pilot_create_texture("gat joel", TEXTURES_SOURCE_DIR "gato-joel.png");

  s32 p_id = ren_create_program_from_files(
      "default", SHADERS_SOURCE_DIR "vertex_texture.glsl",
      SHADERS_SOURCE_DIR "fragment_texture.glsl");

  s32 e1 = ren_create_entity("cube-1");
  s32 cube = ren_primitive_create_cube("cube");
  s32 m_id = ren_create_material("default-mat");
  ren_material_set_program(m_id, p_id);
  ren_material_set_texture(m_id, texture_id);

  ren_entity_add_component(e1, COMPONENT_MATERIAL, m_id);
  ren_entity_add_component(e1, COMPONENT_OBJECT, cube);

  s32 p_grid = ren_create_program_from_files(
      "grid", SHADERS_SOURCE_DIR "vertex_grid.glsl",
      SHADERS_SOURCE_DIR "fragment_grid.glsl");
  s32 m_grid = ren_create_material("grid-mat");
  ren_material_set_program(m_grid, p_grid);
  vec3 grid_line = {1.0f, 1.0f, 1.0f};
  ren_bind_program(programs[p_grid].id);
  ren_program_set_vec3(programs[p_grid].id, "u_line", grid_line);
  ren_program_set_f32(programs[p_grid].id, "u_cell", 0.5f);
  ren_program_set_f32(programs[p_grid].id, "u_density", 0.20f);
  ren_program_set_f32(programs[p_grid].id, "u_major_every", 8.0f);
  ren_program_set_f32(programs[p_grid].id, "u_major_width", 1.0f);
  ren_bind_program(0);
  ren_material_set_transparent(m_grid, 1);

  s32 e_floor = ren_create_entity("floor");
  s32 floor = ren_primitive_create_plane("floor");
  objects[floor].scale[0] = 50.0f;
  objects[floor].scale[2] = 50.0f;
  ren_entity_add_component(e_floor, COMPONENT_MATERIAL, m_grid);
  ren_entity_add_component(e_floor, COMPONENT_OBJECT, floor);
};

void pilot_create_targets() {

  g_gl_target = iom_create_target();
  g_ui_target = iom_create_target();

  struct rect_T b_ui = g_cfg.ui_bounds;
  iom_set_target(g_ui_target, b_ui, 1, pilot_sdl_ui_callback);
  iom_target_set_flag(g_ui_target, TARGET_CALLBACK_ALWAYS);

  struct rect_T b_gl = {.x = 0, .y = 0, .w = 0, .h = 0};
  iom_set_target(g_gl_target, b_gl, 2, pilot_sdl_gl_callback);

  g_popup_target = iom_create_target();
  iom_set_target(g_popup_target, b_gl, -1, pilot_sdl_ui_callback);
  iom_target_set_flag(g_popup_target, TARGET_CALLBACK_NEVER);
}

void pilot_popup_show(s32 show) {
  if (!g_popup_target)
    return;
  if (show) {
    iom_target_z(g_popup_target, 3);
  } else {
    iom_resize_target(g_popup_target, (struct rect_T){0, 0, 0, 0});
    iom_target_z(g_popup_target, -1);
  }
}

void pilot_popup_size(s32 x, s32 y, s32 w, s32 h) {
  if (!g_popup_target)
    return;
  iom_resize_target(g_popup_target, (struct rect_T){x, y, w, h});
  iom_target_z(g_popup_target, 3);
}
