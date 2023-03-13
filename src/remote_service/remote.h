/*
 * Copyright (c) 2023 James B. Greer
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __REMOTE_H_
#define __REMOTE_H_

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/uuid.h>

/** @brief UUID of the Remote Service. **/
#define BT_UUID_REMOTE_SERV_VAL BT_UUID_128_ENCODE(0xe9ea0001, 0xe19b, 0x482d, 0x9293, 0xc7907585fc48)
#define BT_UUID_REMOTE_SERVICE  BT_UUID_DECLARE_128(BT_UUID_REMOTE_SERV_VAL)

/** @brief UUID of button characteristic */
#define BT_UUID_REMOTE_BUTTON_CHRC_VAL BT_UUID_128_ENCODE(0xe9ea0002, 0xe19b, 0x482d, 0x9293, 0xc7907585fc48)
#define BT_UUID_REMOTE_BUTTON_CHRC  BT_UUID_DECLARE_128(BT_UUID_REMOTE_BUTTON_CHRC_VAL)

/** @brief UUID of message characteristic */
#define BT_UUID_REMOTE_MESSAGE_CHRC_VAL BT_UUID_128_ENCODE(0xe9ea0003, 0xe19b, 0x482d, 0x9293, 0xc7907585fc48)
#define BT_UUID_REMOTE_MESSAGE_CHRC  BT_UUID_DECLARE_128(BT_UUID_REMOTE_MESSAGE_CHRC_VAL)

enum bt_button_notifications_enabled {
    BT_BUTTON_NOTIFICATIONS_ENABLED,
    BT_BUTTON_NOTIFICATIONS_DISABLE,
};

struct bt_remote_service_cb {
    void (*notify_changed)(enum bt_button_notifications_enabled status);
    void (*data_received)(struct bt_conn *conn, const uint8_t *const data, uint16_t len);
};

int bt_init(struct bt_conn_cb *bt_cb, struct bt_remote_service_cb *remote_cb);
void set_button_value(uint8_t btn_val);
int send_button_notification(struct bt_conn *conn, uint8_t value);

#endif  /* __REMOTE_H_ */