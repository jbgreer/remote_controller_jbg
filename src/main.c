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

/*  BLE Advertisement options and min/max intervals */
#define BT_LE_ADV_INT_MIN		6399
#define BT_LE_ADV_INT_MAX		6401
static struct bt_le_adv_param bt_param = BT_LE_ADV_PARAM_INIT(BT_LE_ADV_OPT_CONNECTABLE, BT_LE_ADV_INT_MIN, BT_LE_ADV_INT_MAX, NULL);

/* blink LED 1 every 1000 msecs when running */
#define RUN_STATUS_LED			DK_LED1
#define RUN_LED_BLINK_INTERVAL	1000
/* enable LED 2 on bluetooth connection */
#define CONN_STATUS_LED			DK_LED2

/* forward declarations for callback array */
void on_connected(struct bt_conn *conn, uint8_t err);
void on_disconnected(struct bt_conn *conn, uint8_t err);
void on_notify_changed(enum bt_button_notifications_enabled status);
void on_data_received(struct bt_conn *conn, const uint8_t *const data, uint16_t len);

/* for bluetooth callbacks on connect, disconnect events */
struct bt_conn_cb bluetooth_callbacks = {
	.connected 	= on_connected,
	.disconnected 	= on_disconnected,
};

struct bt_remote_service_cb remote_callbacks = {
	.notify_changed = on_notify_changed,
	.data_received = on_data_received,
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

void on_notify_changed(enum bt_button_notifications_enabled status)
{
    if (status == BT_BUTTON_NOTIFICATIONS_ENABLED) {
        LOG_INF("on_notify_changed: Notifications enabled");
    } else {
        LOG_INF("on_notify_changed: Notifications disabled");
    }
}

void on_data_received(struct bt_conn *conn, const uint8_t *const data, uint16_t len)
{
    uint8_t temp_str[len+1];
    memcpy(temp_str, data, len);
    temp_str[len] = 0x00;

    LOG_INF("on_data_received: Received data on conn %p. Len: %d", (void *)conn, len);
    LOG_INF("Data: %s", temp_str);
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
		LOG_INF("button_handler: button pressed>%d<", button_pressed);
		set_button_value(button_pressed);
		int ret = send_button_notification(current_conn, button_pressed);
		if (ret) {
			LOG_ERR("button_handler: no notification >%d<", ret);
		}
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
	/* ret = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));  */
	ret = bt_init(&bt_param, &bluetooth_callbacks, &remote_callbacks);
	if (ret) {
		LOG_ERR("main: can't initialize bt >%d<", ret);
	} else {

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
