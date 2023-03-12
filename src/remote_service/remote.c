/*
 * Copyright (c) 2023 James B. Greer
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "remote.h"

#define LOG_MODULE_NAME			ble
LOG_MODULE_REGISTER(LOG_MODULE_NAME);


int ble_init(void) {
    int ret = 0;
    LOG_INF("ble_init >%d<", ret);

    return ret;
}