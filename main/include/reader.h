// The author disclaims copyright to this source code.
#ifndef _READER_H_
#define _READER_H_

/**
 * @file
 * FreeRTOS Reader task.
 */

#include "buffer.h"

typedef struct reader_config_t {
	buffer_handle_t buffer_handle;
} reader_config_t;

void reader_task(void *pvParameters);

#endif
