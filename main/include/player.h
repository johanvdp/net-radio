// The author disclaims copyright to this source code.
#ifndef _PLAYER_H_
#define _PLAYER_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "buffer.h"
#include "vs1053.h"

/**
 * @file
 * FreeRTOS Player task.
 */

typedef struct player_config_t {
	buffer_handle_t buffer_handle;
	vs1053_handle_t vs1053_handle;
} player_config_t;

void player_task(void *config);

#endif
