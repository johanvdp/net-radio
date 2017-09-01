// The author disclaims copyright to this source code.
#ifndef _TEST_MEM_H_
#define _TEST_MEM_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "spi_mem.h"

/**
 * @file
 * FreeRTOS MEM test task.
 */

typedef struct test_mem_config_t {
	spi_mem_handle_t spi_mem_handle;
} test_mem_config_t;

void test_mem_task(void *pvParameters);

#endif
