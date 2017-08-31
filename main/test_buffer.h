// The author disclaims copyright to this source code.
#ifndef _TEST_BUFFER_H_
#define _TEST_BUFFER_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "buffer.h"

/**
 * @file
 * FreeRTOS Buffer test task.
 */

typedef struct test_buffer_config_t {
	buffer_handle_t buffer_handle;
} test_buffer_config_t;

void test_buffer_task(void *pvParameters);

#endif
