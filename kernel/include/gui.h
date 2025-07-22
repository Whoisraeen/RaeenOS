#ifndef GUI_H
#define GUI_H

#include "core/include/error.h"

error_t graphics_init(void);
error_t gui_init(void);
void gui_dump_windows(void);

#endif