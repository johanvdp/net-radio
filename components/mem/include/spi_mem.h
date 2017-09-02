// The author disclaims copyright to this source code.
#ifndef _SPI_MEM_H_
#define _SPI_MEM_H_

/**
 * @file
 * Driver to access memory chip (23LC1024, and similar) using the SPI bus.
 */

#include "driver/spi_master.h"

typedef enum spi_mem_mode_t {
	/** Transfer data per byte. */
	SPI_MEM_MODE_BYTE = 0x00,
	/** Transfer data per page. */
	SPI_MEM_MODE_PAGE = 0x80,
	/** Transfer data sequentially. */
	SPI_MEM_MODE_SEQUENTIAL = 0x40
} spi_mem_mode_t;

typedef struct spi_mem_config_t {
	spi_host_device_t host;
	int clock_speed_hz;
	int spics_io_num;
	int total_bytes;
	int number_of_pages;
	int number_of_bytes_page;
} spi_mem_config_t;

/**
 * Even though this data is 'public'.
 * Do not shoot yourself in the foot by changing this data.
 */
typedef struct spi_mem_t {
	spi_host_device_t host;
	spi_device_handle_t device_command;
	spi_device_handle_t device_data;
	int clock_speed_hz;
	int spics_io_num;
	int total_bytes;
	int number_of_pages;
	int number_of_bytes_page;
} spi_mem_t;

typedef struct spi_mem_t *spi_mem_handle_t;

/**
 * @brief Begin using the component.
 * @param config The configuration to use.
 * @param handle The created component handle.
 */
void spi_mem_begin(spi_mem_config_t config, spi_mem_handle_t *handle);

/**
 * @brief End using the component.
 * @param handle Component handle.
 */
void spi_mem_end(spi_mem_handle_t handle);

/**
 * @brief READ 0000 0011 0x03 Read data from memory array beginning at selected address.
 * Read memory one byte at a time.
 * Assumes the memory device is already in the correct mode.
 * @param handle Component handle.
 * @param address Memory address.
 * @return The value read.
 */
uint8_t spi_mem_read_byte(spi_mem_handle_t handle, uint32_t address);

/**
 * @brief READ 0000 0011 0x03 Read data from memory array beginning at selected address.
 * Read memory one page at a time.
 * Assumes the memory device is already in the correct mode.
 * @param handle Component handle.
 * @param address Memory address.
 * @param data Target for values read.
 */
void spi_mem_read_page(spi_mem_handle_t handle, uint32_t address, uint8_t *data);

/**
 * @brief READ 0000 0011 0x03 Read data from memory array beginning at selected address.
 * Read memory sequentially.
 * Assumes the memory device is already in the correct mode.
 * @param handle Component handle.
 * @param address Memory address.
 * @param length The number of values read.
 * @param data Target for values read.
 */
void spi_mem_read(spi_mem_handle_t handle, uint32_t address, uint32_t length, uint8_t *data);

/**
 * @brief WRITE 0000 0010 0x02 Write data to memory array beginning at selected address.
 * Write memory one byte at a time.
 * Assumes the memory device is already in the correct mode.
 * @param handle Component handle.
 * @param address Memory address.
 * @param data The value written.
 */
void spi_mem_write_byte(spi_mem_handle_t handle, uint32_t address, uint8_t data);

/**
 * @brief WRITE 0000 0010 0x02 Write data to memory array beginning at selected address.
 * Write memory one page at a time.
 * Assumes the memory device is already in the correct mode.
 * @param handle Component handle.
 * @param address Memory address.
 * @param data Source of values written.
 */
void spi_mem_write_page(spi_mem_handle_t handle, uint32_t address, uint8_t *data);

/**
 * @brief WRITE 0000 0010 0x02 Write data to memory array beginning at selected address.
 * Write memory sequentially.
 * Assumes the memory device is already in the correct mode.
 * @param handle Component handle.
 * @param address Memory address.
 * @param length The number of values written.
 * @param data Source of values written.
 */
void spi_mem_write(spi_mem_handle_t handle, uint32_t address, uint32_t length, uint8_t *data);

/**
 * @brief EDIO 0011 1011 0x3B Enter Dual I/O access (enter SDI bus mode).
 * @param handle Component handle.
 */
void spi_mem_enter_dual_io_access(spi_mem_handle_t handle);

/**
 * @brief EQIO 0011 1000 0x38 Enter Quad I/O access (enter SQI bus mode).
 * @param handle Component handle.
 */
void spi_mem_enter_quad_io_access(spi_mem_handle_t handle);

/**
 * @brief RSTIO 1111 1111 0xFF Reset Dual and Quad I/O access (revert to SPI bus mode).
 * @param handle Component handle
 */
void spi_mem_reset_io_access(spi_mem_handle_t handle);

/**
 * @brief RDMR 0000 0101 0x05 Read Mode Register.
 * This method exists but I have never seen it working YMMV.
 * @param handle SPI device handle.
 */
spi_mem_mode_t spi_mem_read_mode_register(spi_mem_handle_t handle);

/**
 * @brief WRMR 0000 0001 0x01 Write Mode Register.
 * @param handle Component handle.
 * @param mode Access mode.
 */
void spi_mem_write_mode_register(spi_mem_handle_t handle, spi_mem_mode_t mode);

#endif
