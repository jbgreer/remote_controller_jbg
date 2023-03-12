/*
 * Copyright (c) 2023 James B. Greer
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#define LOG_MODULE_NAME		app
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

void main(void)
{
	LOG_INF("Hello World! %s\n", CONFIG_BOARD);
}
