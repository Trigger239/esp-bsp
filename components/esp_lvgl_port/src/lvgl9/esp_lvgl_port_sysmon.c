/*
 * SPDX-FileCopyrightText: 2026 Arseniy Virachev <trigger239@users.noreply.github.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"

static const char *TAG = "LVGL";

/*******************************************************************************
* Types definitions
*******************************************************************************/

typedef struct lvgl_port_ctx_s {
    uint64_t idle_time_sum;
    uint64_t non_idle_time_sum;
    uint64_t task_switch_timestamp[configNUM_CORES];
    bool idle_task_running[configNUM_CORES];
} lvgl_port_sysmon_ctx_t;

/*******************************************************************************
* Local variables
*******************************************************************************/
static portMUX_TYPE lvgl_port_sysmon_lock = portMUX_INITIALIZER_UNLOCKED;
static lvgl_port_sysmon_ctx_t lvgl_port_sysmon_ctx;

/*******************************************************************************
* Public API functions
*******************************************************************************/

void lvgl_port_task_switch_in(const char * name)
{
    BaseType_t core_id = xPortGetCoreID();
    if (strncmp(name, "IDLE", 4)) {
        lvgl_port_sysmon_ctx.idle_task_running[core_id] = false;
    } else {
        lvgl_port_sysmon_ctx.idle_task_running[core_id] = true;
    }
    lvgl_port_sysmon_ctx.task_switch_timestamp[core_id] = esp_timer_get_time();
}

void lvgl_port_task_switch_out(void)
{
    BaseType_t core_id = xPortGetCoreID();
    uint64_t elaps = esp_timer_get_time() - lvgl_port_sysmon_ctx.task_switch_timestamp[core_id];
    portENTER_CRITICAL_SAFE(&lvgl_port_sysmon_lock);
    if (lvgl_port_sysmon_ctx.idle_task_running[core_id]) {
        lvgl_port_sysmon_ctx.idle_time_sum += elaps;
    } else {
        lvgl_port_sysmon_ctx.non_idle_time_sum += elaps;
    }
    portEXIT_CRITICAL_SAFE(&lvgl_port_sysmon_lock);
}

uint32_t lvgl_port_get_idle_percent(void)
{
    portENTER_CRITICAL_SAFE(&lvgl_port_sysmon_lock);
    uint64_t idle_time_sum = lvgl_port_sysmon_ctx.idle_time_sum;
    uint64_t non_idle_time_sum = lvgl_port_sysmon_ctx.non_idle_time_sum;
    lvgl_port_sysmon_ctx.non_idle_time_sum = 0;
    lvgl_port_sysmon_ctx.idle_time_sum = 0;
    portEXIT_CRITICAL_SAFE(&lvgl_port_sysmon_lock);

    if (non_idle_time_sum + idle_time_sum == 0) {
        ESP_LOGW(TAG, "Not enough time elapsed to provide idle percentage");
        return 0;
    }

    uint32_t pct = (idle_time_sum * 100) / (idle_time_sum + non_idle_time_sum);

    return pct;
}
