// The author disclaims copyright to this source code.
#ifndef _BUFFER_H_
#define _BUFFER_H_

/**
 * @file
 * Ring buffer.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "spi_ram.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_heap_caps.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

struct buffer_t {
	spi_ram_handle_t spi_ram_handle;
	uint32_t buffer_read_addr;
	uint32_t buffer_write_addr;
};

typedef struct buffer_t *buffer_handle_t;

/**
 * Check number of bytes that can be pulled from the buffer.
 * @return Number of bytes.
 */
uint32_t buffer_available(buffer_handle_t handle);
/**
 * Check number of bytes that can be pushed into the buffer.
 * @return Number of bytes.
 */
uint32_t buffer_free(buffer_handle_t handle);
/**
 * Push a number of bytes into the buffer.
 * @param data Source of data.
 * @param length Number of bytes.
 */
void buffer_push(buffer_handle_t handle, uint8_t *data, uint32_t length);
/**
 * Pull a number of bytes from the buffer.
 * @param data Target of data.
 * @param length Number of bytes.
 */
void buffer_pull(buffer_handle_t handle, uint8_t *page, uint32_t length);
/**
 * Log buffer configuration.
 */
void buffer_log_configuration();
/**
 * Begin usage.
 */
void buffer_begin(spi_host_device_t host, buffer_handle_t *handle);
/**
 * End usage.
 */
void buffer_end(buffer_handle_t handle);


#endif
