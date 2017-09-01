#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "sdkconfig.h"

#include "driver/gpio.h"
#include "mdns.h"

#include "lwip/api.h"
#include "lwip/err.h"
#include "lwip/netdb.h"

static const char http_header[] = "HTTP/1.1 200 OK\nContent-type: text/html\n\n";
static const char http_content[] = "<html><head></head><body>Hello world!</body></html>";

static const char* TAG = "webserver.c";

static void http_server_netconn_serve(struct netconn *conn) {
	ESP_LOGD(TAG, ">http_server_netconn_serve")
	struct netbuf *inbuf;
	char *buf;
	u16_t buflen;
	err_t err;

	err = netconn_recv(conn, &inbuf);

	if (err == ERR_OK) {

		netbuf_data(inbuf, (void**) &buf, &buflen);

		// extract the first line, with the request
		char *line = strtok(buf, "\n");

		if (line) {

			if (strstr(line, "GET / ")) {
				netconn_write(conn, http_header, sizeof(http_header) - 1, NETCONN_NOCOPY);
				netconn_write(conn, http_content, sizeof(http_content) - 1, NETCONN_NOCOPY);
			} else {
				ESP_LOGD(TAG, "Unknown: %s\n", line);
			}
		} else {
			ESP_LOGD(TAG, "Expected newline")
		}
	}

	// close the connection and free the buffer
	netconn_close(conn);
	netbuf_delete(inbuf);
	ESP_LOGD(TAG, "<http_server_netconn_serve")
}

void webserver_task(void *pvParameters) {
	ESP_LOGI(TAG, ">webserver_task");

	struct netconn *conn, *newconn;
	err_t err;
	conn = netconn_new(NETCONN_TCP);
	netconn_bind(conn, NULL, 80);
	netconn_listen(conn);
	ESP_LOGD(TAG, "netconn_bind: %d", 80);

	do {
		err = netconn_accept(conn, &newconn);
		if (err == ERR_OK) {
			http_server_netconn_serve(newconn);
			netconn_delete(newconn);
		}
	} while (err == ERR_OK);

	netconn_close(conn);
	netconn_delete(conn);
	ESP_LOGE(TAG, "<webserver_task");
}
