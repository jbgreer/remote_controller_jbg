/*
 * Copyright (c) 2023 James B. Greer
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "remote.h"

/* unique log identifier for this file */
#define LOG_MODULE_NAME			ble
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

/* SEMAPHORE for coordinating bt_ready state from bt_enable */
static K_SEM_DEFINE(bt_init_ok, 0, 1);
static uint8_t button_value;

/* for use in advertisement */
#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME)-1)

/* normal advertisement data */
static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN)
};

/* scan response advertisement data */
static const struct bt_data sd[] = {
    BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_REMOTE_SERV_VAL),
};

/* structure for remote service callbacks */
static struct bt_remote_service_cb remote_service_callbacks;

/* forward declarations */
static ssize_t read_button_characteristic_cb(struct bt_conn *conn, const struct bt_gatt_attr *attr, 
    void *buf, uint16_t len, uint16_t offset);
void button_chrc_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value);
static ssize_t on_write(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf, uint16_t len, uint16_t offset, uint8_t flags);

/* macro to define a bt service and characteristics */
BT_GATT_SERVICE_DEFINE(remote_srv,
BT_GATT_PRIMARY_SERVICE(BT_UUID_REMOTE_SERVICE),
    BT_GATT_CHARACTERISTIC(BT_UUID_REMOTE_BUTTON_CHRC,
        BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
        BT_GATT_PERM_READ,
        read_button_characteristic_cb, NULL, NULL),
    BT_GATT_CCC(button_chrc_ccc_cfg_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
    BT_GATT_CHARACTERISTIC(BT_UUID_REMOTE_MESSAGE_CHRC,
        BT_GATT_CHRC_WRITE_WITHOUT_RESP,
        BT_GATT_PERM_WRITE,
        NULL, on_write, NULL),
);

/** @brief read_button_characteristic_cb - callback to fetch value of last button press
 * 
 */
static ssize_t read_button_characteristic_cb(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 void *buf, uint16_t len, uint16_t offset) {
	return bt_gatt_attr_read(conn, attr, buf, len, offset, &button_value,
				 sizeof(button_value));
}

/** @brief button_chrc_ccc_cfg_change: callback used as part of notification of button press change
 */
void button_chrc_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value) {
   bool notify_enabled = (value == BT_GATT_CCC_NOTIFY);
   LOG_INF("button_chrc_ccc_cfg_changed: notifications %s", notify_enabled? "enabled":"disabled");
}

void on_sent(struct bt_conn *conn, void *user_data) {
    ARG_UNUSED(user_data);
    LOG_INF("on_sent: Notification sent on connection %p", (void *)conn);
}

static ssize_t on_write(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                        const void *buf, uint16_t len, uint16_t offset, uint8_t flags) {
    LOG_INF("on_write: Received data, handle %d, conn %p",
        attr->handle, (void *)conn);

    if (remote_service_callbacks.data_received) {
        remote_service_callbacks.data_received(conn, buf, len);
    }
    return len;
}

/* Remote controller functions */
int send_button_notification(struct bt_conn *conn, uint8_t value)
{
    struct bt_gatt_notify_params params = {0};
    const struct bt_gatt_attr *attr = &remote_srv.attrs[2];

    params.attr = attr;
    params.data = &value;
    params.len = 1;
    params.func = on_sent;

    int err = bt_gatt_notify_cb(conn, &params);
    return err;
}

/*  
 * bt_ready: handler for bt_enable
*/
static void bt_ready(int ret) {
    if (ret) {
        LOG_ERR("bt_ready >%d<", ret);
    }
    k_sem_give(&bt_init_ok);
}

void set_button_value(uint8_t btn_val) {
    LOG_INF("set_button_value >%d<", btn_val);
    button_value = btn_val;
}

/* 
 * bt_init
*/
int bt_init(struct bt_conn_cb *bt_cb, struct bt_remote_service_cb *remote_cb) {
    int ret = 0;
    LOG_INF("bt_init >%d<", ret);

    /* confirm callback array exists */
    if (bt_cb == NULL || remote_cb == NULL) {
        return NRFX_ERROR_NULL;
    }

    /* register bluetooth call backs for connect/disconnect events */
    bt_conn_cb_register(bt_cb);

    remote_service_callbacks.notify_changed = remote_cb->notify_changed;
    remote_service_callbacks.data_received = remote_cb->data_received;

    /* enable and confirm readiness of bluetooth */
    ret = bt_enable(bt_ready);
    if (ret) {
        LOG_ERR("bt_enable: >%d<", ret);
        return ret;
    }
    k_sem_take(&bt_init_ok, K_FOREVER);
    LOG_INF("bt_enable >%d<", ret);

    /* begin advertising */
    ret = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
    if (ret){
        LOG_ERR("bt_le_adv_start >%d<", ret);
        return ret;
    }
    LOG_INF("bt_le_adv_start >%d<", ret);

    return ret;
}