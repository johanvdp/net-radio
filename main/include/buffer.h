// The author disclaims copyright to this source code.
#ifndef _BUFFER_H_
#define _BUFFER_H_

/**
 * @file
 * Ring buffer.
 */

#include "spi_mem.h"

struct buffer_t {
	spi_mem_handle_t spi_mem_handle;
	uint32_t size;
	uint32_t mask;
	uint32_t read_addr;
	uint32_t write_addr;
	SemaphoreHandle_t mutex;
	uint32_t push_bytes;
	uint32_t pull_bytes;
	uint32_t push_count;
	uint32_t pull_count;
};
typedef struct buffer_config_t {
	spi_mem_handle_t spi_mem_handle;
	/**
	 * buffer algorithm only works when size is a power of two.
	 */
	uint32_t size;
} buffer_config_t;
typedef struct buffer_t *buffer_handle_t;

/**
 * @brief Log configuration.
 * @param config The configuration.
 */
void buffer_log_config(buffer_config_t config);

/**
 * @brief Log current state.
 * @param handle Component handle.
 */
void buffer_log(buffer_handle_t handle);

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
 * @param length Number of bytes.
 * @param data Target of data.
 */
void buffer_pull(buffer_handle_t handle, uint32_t length, uint8_t *data);

/**
 * @brief Begin buffer usage.
 * @param config Buffer configuration.
 * @param handle Created buffer handle.
 */
void buffer_begin(buffer_config_t config, buffer_handle_t *handle);

/**
 * @brief End buffer usage.
 * @param handle Buffer handle.
 */
void buffer_end(buffer_handle_t handle);

/**
 * @brief Clear buffer.
 * @param handle Buffer handle.
 */
void buffer_reset(buffer_handle_t handle);

#endif
