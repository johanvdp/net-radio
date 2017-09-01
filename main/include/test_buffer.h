// The author disclaims copyright to this source code.
#ifndef _TEST_BUFFER_H_
#define _TEST_BUFFER_H_

/**
 * @file
 * Buffer test.
 */

#include "buffer.h"
#include "esp_err.h"

typedef struct test_buffer_config_t {
	buffer_handle_t buffer_handle;
} test_buffer_config_t;

esp_err_t test_buffer(test_buffer_config_t config);

#endif
