#include "ui.h"

#include "pilot.h"

#include "global.h"

extern "C" {
#include "io_manager.h"
#include "render.h"
#include "types.h"
}

int main(int argc, char *argv[]) {

  pilot_ui_log(PILOT_UI_INFO, "Starting...");

  pilot_ui_log(PILOT_UI_INFO, "Initializing UI...");
  if (ui_init(argc, argv) != 0) {
    return -1;
  }
  pilot_ui_log(PILOT_UI_INFO, "UI initialized.");

  pilot_set_msg_calls();

  pilot_cfg_init();
  pilot_ui_log(PILOT_UI_INFO, "Initializing IOM...");
  ui_set_ui_texture_callback(&pilot_ui_texture_clbk);
  ui_set_cursor_callback(&pilot_ui_cursor_clbk);
  ui_resize_window(g_cfg.win_w, g_cfg.win_h);
  iom_init();
  pilot_ui_log(PILOT_UI_INFO, "IOM initialized.");

  pilot_create_targets();

  pilot_ui_log(PILOT_UI_INFO, "Initializing render...");
  ren_init();
  pilot_ui_log(PILOT_UI_INFO, "render initialized.");

  pilot_init_scene();

  pilot_ui_log(PILOT_UI_INFO,
               "All initialized correctly. Starting main loop...");

  while (!iom_can_close()) {
    iom_poll_events();
    pilot_ui_log_flush();
    ren_draw_frame();
    ui_message_loop();
  }

  ui_close();
  iom_quit();

  return 0;
};
