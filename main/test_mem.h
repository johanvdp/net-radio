// The author disclaims copyright to this source code.
#ifndef _TEST_MEM_H_
#define _TEST_MEM_H_

/**
 * @file
 * FreeRTOS MEM test task.
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "sdkconfig.h"
#include "mem.h"

void test_mem_log_configuration();
void test_mem_task(void *ignore);

#endif
