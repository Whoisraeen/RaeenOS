#ifndef DRIVERS_H
#define DRIVERS_H

#include "core/include/error.h"

error_t device_manager_init(void);
error_t vga_init(void);
error_t usb_init(void);

#endif