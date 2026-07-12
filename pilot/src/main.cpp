#include "SDL3/SDL_events.h"
#include "ui.h"

extern "C" {
#include "camera.h"
#include "entity.h"
#include "global.h" // Assuming 'state' is declared here as an extern int
#include "io_manager.h"
#include "log.h"
#include "object.h"
#include "render.h"
#include "shader.h"
}

extern int g_app_state;
const int UI_STATE_READY_TO_EXIT = 2;

void sdl_callback(SDL_Event *e) {
  switch (e->type) {
  case SDL_EVENT_QUIT:
    state = 0;
    break;
  case SDL_EVENT_KEY_DOWN:
    switch (e->key.key) {
    case SDLK_ESCAPE:
      state = 0;
      break;
    }
    break;
  default:
    break;
  }
}

void start_camera() {
  ren_set_camera_aspect_ratio_wh(800, 600);
  ren_set_camera_planes(0.1f, 100.0f);
  ren_set_camera_projection(PERSPECTIVE);
  ren_set_camera_fov(1.3);
  ren_camera_set_position(0.0, 0.0, 5.0);
  ren_camera_set_rotation(0.0, 0.0, 0.0);
}

int main(int argc, char *argv[]) {

  //  if (ui_start(argc, argv) != 0) {
  //    return -1;
  //  }

  iom_init();
  ren_init();

  iom_set_event_callback(sdl_callback);
  start_camera();

  s32 p_id = ren_create_program_from_files(SHADERS_SOURCE_DIR "vertex.glsl",
                                           SHADERS_SOURCE_DIR "fragment.glsl");
  s32 e1 = ren_create_entity();
  s32 cube = ren_primitive_create_cube();
  ren_entity_add_component(e1, COMPONENT_OBJECT, cube);
  ren_entity_add_component(e1, COMPONENT_PROGRAM, p_id);

  while (state) {
    iom_poll_events();
    ren_draw_frame();

    //  ui_message_loop();
  }

  INFO("QUITTING");

  iom_quit();
  // ui_close();

  return 0;
};
