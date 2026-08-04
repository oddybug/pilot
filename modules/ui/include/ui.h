#include "types.h"

extern s32 ui_start(int argc, char *argv[]);

extern s32 ui_get_texture_id();

extern void ui_message_loop();

extern bool ui_can_close();

extern void ui_close();

extern void ui_set_ui_texture_callback(void (*clbk)(u8 *buffer, u32 width,
                                                    u32 height));

extern void ui_resize_window(u32 width, u32 height);

void ui_close_browsers();
