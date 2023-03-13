/*
 * Copyright (c) 2023 James B. Greer
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <dk_buttons_and_leds.h>

#include "remote.h"

/* logging for this file */
#define LOG_MODULE_NAME			app
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

/* blink LED 1 every 1000 msecs when running */
#define RUN_STATUS_LED			DK_LED1
#define RUN_LED_BLINK_INTERVAL	1000
/* enable LED 2 on bluetooth connection */
#define CONN_STATUS_LED			DK_LED2

/* forward declarations for callback array */
void on_connected(struct bt_conn *conn, uint8_t err);
void on_disconnected(struct bt_conn *conn, uint8_t err);

/* for bluetooth callbacks on connect, disconnect events */
struct bt_conn_cb bluetooth_callbacks = {
	.connected 	= on_connected,
	.disconnected 	= on_disconnected,
};

/* current bluetooth connection, if any */
static struct bt_conn *current_conn;

/** @brief on_connected: callback function for bluetooth connection event
 *  @param conn (struct bt_conn *)
 *  @param err (uint8_t)
 *  @return void
 */
void on_connected(struct bt_conn *conn, uint8_t err) {
	if (err) {
		LOG_ERR("on_connected <%d>", err);
		return;
	}
	LOG_INF("on_connected <%d>", err);
	/* capture current connection and turn on connection LED */
	current_conn = bt_conn_ref(conn);
	dk_set_led_on(CONN_STATUS_LED);
}

/** @brief on_disconnected: callback function for bluetooth disconnection event
 *  @param conn (struct bt_conn *)
 *  @param err (uint8_t)
 *  @return void
 */
void on_disconnected(struct bt_conn *conn, uint8_t err) {
	LOG_INF("on_disconnected <%d>", err);
	/* turn off connection LED and reset current connection if set */
	dk_set_led_off(CONN_STATUS_LED);
	if (current_conn) {
		bt_conn_unref(current_conn);
		current_conn = NULL;
	}
}

/* 
 * button_handler for when buttons are pressed
 */
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
		set_button_value(button_pressed);
	}
}


/*
 * configure_dk_buttons_leds
 */
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


/*
 * main
 */
void main(void)
{
	LOG_INF("Startup:>%s<", CONFIG_BOARD);
	int ret = 0;

	/* initialize bluetooth */
	ret = bt_init(&bluetooth_callbacks);
	if (!ret) {

		/* setup leds and buttons */
		ret = configure_dk_buttons_leds();
		if (!ret) {
			int blink = 0;

			/* main loop - toggle status LED and sleep */
			for (;;) {
				dk_set_led(RUN_STATUS_LED, (blink++)%2);
				k_sleep(K_MSEC(RUN_LED_BLINK_INTERVAL));
			}
		} 
	}
	LOG_ERR("TERMINATING");
}
