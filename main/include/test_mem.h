// The author disclaims copyright to this source code.
#ifndef _TEST_MEM_H_
#define _TEST_MEM_H_

/**
 * @file
 * FreeRTOS MEM test task.
 */

#include "spi_mem.h"
#include "esp_err.h"

typedef struct test_mem_config_t {
	spi_mem_handle_t spi_mem_handle;
} test_mem_config_t;

esp_err_t test_mem(test_mem_config_t config);

#endif
