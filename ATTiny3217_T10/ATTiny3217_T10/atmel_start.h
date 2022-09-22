#ifndef ATMEL_START_H_INCLUDED
#define ATMEL_START_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include "include/driver_init.h"
#include "include/atmel_start_pins.h"

#include "touch.h"

/**
 * Initializes MCU, drivers and middleware in the project
 **/
void atmel_start_init(void);
void read_BLE_data(void);
void write_data(char *buffer);
void rgb_button(int i);
void touch_detected(int i);
void slider(int i);
void slider_status(int i);
void inc_dec(int curr_button_val,int prev,int i);



#ifdef __cplusplus
}
#endif
#endif
