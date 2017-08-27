// The author disclaims copyright to this source code.
#ifndef _TEST_DSP_H_
#define _TEST_DSP_H_

/**
 * @file
 * FreeRTOS DSP test task.
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "sdkconfig.h"
#include "dsp.h"

void test_dsp_log_configuration();
void test_dsp_task(void *ignore);

#endif
