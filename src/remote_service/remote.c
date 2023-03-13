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

/*  
 * bt_ready: handler for bt_enable
*/
static void bt_ready(int ret) {
    if (ret) {
        LOG_ERR("bt_ready >%d<", ret);
    }
    k_sem_give(&bt_init_ok);
}

/* 
 * bt_init
*/
int bt_init(struct bt_conn_cb *bt_callbacks) {
    int ret = 0;
    LOG_INF("bt_init >%d<", ret);

    /* confirm callback array exists */
    if (bt_callbacks == NULL) {
        return NRFX_ERROR_NULL;
    }

    /* register bluetooth call backs for connect/disconnect events */
    bt_conn_cb_register(bt_callbacks);

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