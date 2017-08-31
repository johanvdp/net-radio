// The author disclaims copyright to this source code.
#ifndef _STATISTICS_H_
#define _STATISTICS_H_

/**
 * @file
 * FreeRTOS Log statistics task.
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "buffer.h"

typedef struct statistics_config_t {
	buffer_handle_t buffer_handle;
} statistics_config_t;

/**
 * @brief The task entry function.
 * This function never returns.
 */
void statistics_task(void *pvParameters);

#endif
