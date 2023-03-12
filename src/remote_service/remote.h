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

int bt_init(void);

#endif  /* __REMOTE_H_ */