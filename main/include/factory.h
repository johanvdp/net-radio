// The author disclaims copyright to this source code.
#ifndef _FACTORY_H_
#define _FACTORY_H_

/**
 * @file
 * Factory.
 */

#include "vs1053.h"
#include "buffer.h"

void factory_mem_create(spi_mem_handle_t *handle);
void factory_dsp_create(vs1053_handle_t *handle);
void factory_buffer_create(spi_mem_handle_t spi_mem_handle, uint32_t size, buffer_handle_t *handle);

#endif
