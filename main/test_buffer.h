// The author disclaims copyright to this source code.
#ifndef _TEST_BUFFER_H_
#define _TEST_BUFFER_H_

#include "buffer.h"
#include "esp_err.h"

/**
 * @file
 * Buffer test.
 */

typedef struct test_buffer_config_t {
	buffer_handle_t buffer_handle;
} test_buffer_config_t;

esp_err_t test_buffer(test_buffer_config_t config);

#endif
