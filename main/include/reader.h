// The author disclaims copyright to this source code.
#ifndef _READER_H_
#define _READER_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "buffer.h"

/**
 * @file
 * FreeRTOS Reader task.
 */

typedef struct reader_config_t {
	buffer_handle_t buffer_handle;
} reader_config_t;

void reader_task(void *pvParameters);

#endif
