// The author disclaims copyright to this source code.
#ifndef _TEST_DSP_H_
#define _TEST_DSP_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "vs1053.h"

/**
 * @file
 * FreeRTOS DSP test task.
 */

typedef struct test_dsp_config_t {
	vs1053_handle_t vs1053_handle;
} test_dsp_config_t;

void test_dsp_task(void *pvParameters);

#endif
