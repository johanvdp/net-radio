// The author disclaims copyright to this source code.
#ifndef _HELLO_H_
#define _HELLO_H_

/**
 * @file
 * FreeRTOS Hello task.
 */

#include "buffer.h"

typedef struct hello_config_t {
	buffer_handle_t buffer_handle;
} hello_config_t;

void hello_task(void *pvParameters);

#endif
