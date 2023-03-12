/*
 * Copyright (c) 2023 James B. Greer
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "remote.h"

#define LOG_MODULE_NAME			ble
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

/* SEMAPHORE FOR BT ready state */
static K_SEM_DEFINE(bt_init_ok, 0, 1);


/*  
 * bt_ready: handler for bt_enable
*/
static void bt_ready(int ret) {
    if (ret) {
        LOG_ERR("bt_ready >%d<", ret);
    }
    k_sem_give(&bt_init_ok);
}

int bt_init(void) {
    int ret = 0;
    LOG_INF("bt_init >%d<", ret);

    /* enable and confirm readiness of bluetooth */
    ret = bt_enable(bt_ready);
    if (ret) {
        LOG_ERR("bt_enable: >%d<", ret);
        return ret;
    }
    k_sem_take(&bt_init_ok, K_FOREVER);

    return ret;
}