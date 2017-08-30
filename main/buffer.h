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
#include "spi_mem.h"
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
	spi_mem_handle_t spi_mem_handle;
	uint32_t buffer_read_addr;
	uint32_t buffer_write_addr;
};

typedef struct buffer_t *buffer_handle_t;

/**
 * @brief Check number of bytes that can be pulled from the buffer.
 * @param handle Buffer handle.
 * @return Number of bytes.
 */
uint32_t buffer_available(buffer_handle_t handle);

/**
 * Check number of bytes that can be pushed into the buffer.
 * @param handle Buffer handle.
 * @return Number of bytes.
 */
uint32_t buffer_free(buffer_handle_t handle);

/**
 * @brief Push a number of bytes into the buffer.
 * @param handle  Buffer handle.
 * @param data Source of data.
 * @param length Number of bytes.
 */
void buffer_push(buffer_handle_t handle, uint8_t *data, uint32_t length);

/**
 * @brief Pull a number of bytes from the buffer.
 *
 * @param handle Buffer handle.
 * @param data Target of data.
 * @param length Number of bytes.
 */
void buffer_pull(buffer_handle_t handle, uint8_t *data, uint32_t length);

/**
 * @brief Begin usage.
 * @param host SPI host number.
 * @param handle Created buffer handle.
 */
void buffer_begin(spi_host_device_t host, buffer_handle_t *handle);
/**
 * @brief End usage.
 * @param handle Buffer handle.
 */
void buffer_end(buffer_handle_t handle);

#endif
