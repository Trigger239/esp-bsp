/*
 * SPDX-FileCopyrightText: 2026 Arseniy Virachev <trigger239@users.noreply.github.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief ESP LVGL port FreeRTOS hooks
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void lvgl_port_task_switch_in(const char *name);
void lvgl_port_task_switch_out(void);

#define traceTASK_SWITCHED_IN() lvgl_port_task_switch_in(pxCurrentTCB[xPortGetCoreID()]->pcTaskName)
#define traceTASK_SWITCHED_OUT() lvgl_port_task_switch_out()

#ifdef __cplusplus
}
#endif
