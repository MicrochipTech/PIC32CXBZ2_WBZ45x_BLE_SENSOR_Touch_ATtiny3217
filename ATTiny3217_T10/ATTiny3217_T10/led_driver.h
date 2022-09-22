/*
 * led_driver.h
 *
 * Created: 18-09-2017 11:45:02
 *  Author: I41681
 */

#ifndef LED_DRIVER_H_
#define LED_DRIVER_H_

#include "touch.h"

#define ENABLE_LED 1u

#if ENABLE_LED == 1u

#include "i2c_master.h"
#include "atmel_start_pins.h"

#define SLAVE_ADDR 0x20

#define LED_BUTTON 0
#define LED_SCROLLER 1

typedef struct tag_led_scroll_t {
	uint8_t value;
	uint8_t dir;
} led_scroll_t;

void init_led_driver(void);
void led_gpio_update(uint8_t data, uint8_t ver_or_hor);
void led_reset(void);
void led_decode_and_update(void);
void uart_output(uint8_t data_byte);
void uart_send_periodic_data(void);
#endif

#endif /* LED_DRIVER_H_ */
