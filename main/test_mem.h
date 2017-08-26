// The author disclaims copyright to this source code.
#ifndef _TEST_MEM_H_
#define _TEST_MEM_H_

/**
 * @file
 * FreeRTOS MEM test task.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_heap_caps.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "mem.h"

void test_mem_log_configuration();
void test_mem_task(void *ignore);

#endif
