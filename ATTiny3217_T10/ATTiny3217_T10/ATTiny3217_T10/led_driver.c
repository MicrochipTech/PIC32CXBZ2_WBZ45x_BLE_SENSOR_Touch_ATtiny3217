/*
 * led_driver.c
 *
 * Created: 18-09-2017 11:44:47
 *  Author: I41681
 */

#include "led_driver.h"
#include "datastreamer.h"
#include "driver_init.h"

#if ENABLE_LED == 1u

uint8_t i2c_write_buf[3];

void I2C_write(uint8_t slvae_address, uint8_t *buffer, uint8_t len)
{
	I2C_0_open(slvae_address);
	I2C_0_set_buffer(buffer, len);
	I2C_0_master_write();
	while (I2C_BUSY == I2C_0_close());
}

void init_led_driver(void)
{
	i2c_write_buf[0] = 0x00; // reg direction
	i2c_write_buf[1] = 0x00; // output
	I2C_write(SLAVE_ADDR, i2c_write_buf, 2);

	i2c_write_buf[0] = 0x14; // reg address
	i2c_write_buf[1] = 0x0;  // all low 0X6F
	I2C_write(SLAVE_ADDR, i2c_write_buf, 2);

	i2c_write_buf[0] = 0x01; // reg direction
	i2c_write_buf[1] = 0x00; // output
	I2C_write(SLAVE_ADDR, i2c_write_buf, 2);

	i2c_write_buf[0] = 0x15; // reg address
	i2c_write_buf[1] = 0x0;  // all low 0X1F
	I2C_write(SLAVE_ADDR, i2c_write_buf, 2);
}

void led_gpio_update(uint8_t data, uint8_t ver_or_hor)
{
	if (ver_or_hor == LED_BUTTON) {
		i2c_write_buf[0] = 0x14; // reg address
		i2c_write_buf[1] = data; // all low
		I2C_write(SLAVE_ADDR, i2c_write_buf, 2);
	} 
	else {
		i2c_write_buf[0] = 0x15; // reg address
		i2c_write_buf[1] = data; // all low
		I2C_write(SLAVE_ADDR, i2c_write_buf, 2);
	}
}

uint8_t blink_code = 0;

void led_decode_and_update()
{
	uint8_t button_mask = 0, slider_mask = 0, scroller_pos = 0;
		for (uint8_t cnt = 0; cnt < 4; cnt++) {
			if (get_sensor_state(cnt) & 0x80) {
				button_mask |= (1 << (3 - cnt));
			}
		}

		if (get_scroller_state(0) & 0x80) {
			scroller_pos = get_scroller_position(0);
			scroller_pos = scroller_pos >> 5;

			slider_mask = (1 << 7);
			while (scroller_pos > 0) {
				slider_mask |= (1 << (7 - (scroller_pos)));
				scroller_pos--;
			}
		}

		button_mask = button_mask << 4;

	led_gpio_update(button_mask, LED_BUTTON);
	led_gpio_update(slider_mask, LED_SCROLLER);
}

void led_reset(void)
{
	led_gpio_update(0, LED_BUTTON);
	led_gpio_update(0, LED_SCROLLER);
}

#endif
