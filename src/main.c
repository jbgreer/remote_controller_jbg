/*
 * Copyright (c) 2023 James B. Greer
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <dk_buttons_and_leds.h>

#include "remote.h"

#define LOG_MODULE_NAME			app
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

#define RUN_STATUS_LED			DK_LED1
#define RUN_LED_BLINK_INTERVAL	1000


void button_handler(uint32_t button_state, uint32_t has_changed) {
	int button_pressed = 0;
	if (has_changed & button_state) {
		switch (has_changed) {
			case DK_BTN1_MSK:
				button_pressed = 1;
				break;
			case DK_BTN2_MSK:
				button_pressed = 2;
				break;
			case DK_BTN3_MSK:
				button_pressed = 3;
				break;
			case DK_BTN4_MSK:
				button_pressed = 4;
				break;
			default:
				break;
		}
		LOG_INF("button pressed>%d<", button_pressed);
	}
}


static int configure_dk_buttons_leds(void) {
	int ret = 0;

	ret = dk_leds_init();
	if (ret) {
		LOG_ERR("dk_leds_init: ret>%d<", ret);
		return ret;
	}

	ret = dk_buttons_init(button_handler);
	if (ret) {
		LOG_ERR("dk_buttons_init: ret>%d<", ret);
		return ret;
	}

	return ret;
}


void main(void)
{
	LOG_INF("Startup:>%s<", CONFIG_BOARD);
	int blink = 0;

	int ret = 0;

	ret = bt_init();
	if (!ret) {

		ret = configure_dk_buttons_leds();
		if (!ret) {

			for (;;) {
				dk_set_led(RUN_STATUS_LED, (blink++)%2);
				k_sleep(K_MSEC(RUN_LED_BLINK_INTERVAL));
			}
		} 
	}
	LOG_ERR("TERMINATING");
}
