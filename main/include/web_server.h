// The author disclaims copyright to this source code.
#ifndef _WEB_SERVER_H_
#define _WEB_SERVER_H_

/**
 * @file
 * Web server task.
 */

#include <stdint.h>

typedef struct web_server_config_t {
	uint16_t port;
} web_server_config_t;

/**
 * FreeRTOS Web server task.
 */
void web_server_task(void *pvUnused);


#endif
