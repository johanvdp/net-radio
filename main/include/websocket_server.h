// The author disclaims copyright to this source code.
#ifndef _WEBSOCKET_SERVER_H_
#define _WEBSOCKET_SERVER_H_

/**
 * @file
 * FreeRTOS WebSocket server task.
 *
 * 1) https://tools.ietf.org/html/rfc6455
 * 2) https://developer.mozilla.org/en-US/docs/Web/API/WebSockets_API/Writing_WebSocket_servers
 *
 * Some shortcuts have been taken:
 * - only one websocket client
 * - web traffic is handled on a separate server (also limited to one client)
 * - short frame payload to avoid masking (see explanation in 2)
 * - the simplest thing that could possibly work with my web client
 */

#include <stdint.h>

typedef struct websocket_server_config_t {
	uint16_t port;
} websocket_server_config_t;

void websocket_server_task(void *pvParameters);
void websocket_process_task(void *pvUnused);

#endif
