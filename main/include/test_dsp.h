// The author disclaims copyright to this source code.
#ifndef _TEST_DSP_H_
#define _TEST_DSP_H_

/**
 * @file
 * DSP test.
 */

#include "vs1053.h"
#include "esp_err.h"

typedef struct test_dsp_config_t {
	vs1053_handle_t vs1053_handle;
} test_dsp_config_t;

esp_err_t test_dsp(test_dsp_config_t config);

#endif
