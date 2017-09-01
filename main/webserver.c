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

/**
 * Request-Line = Method SP Request-URI SP HTTP-Version CRLF
 * one or more header lines
 * one empty line
 */
static const char http_ok[] = "HTTP/1.0 200 OK\nContent-type: text/html\nConnection: Closed\n\n";
/**
 * Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF
 * one or more header lines
 * one empty line
 */
static const char http_bad_request[] = "HTTP/1.0 400 Bad Request\nContent-Length: 0\nConnection: Closed\n\n";
/**
 * Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF
 * one or more header lines
 * one empty line
 */
static const char http_server_error[] = "HTTP/1.0 500 Internal Server Error\nContent-Length: 0\nConnection: Closed\n\n";
/** Content */
static const char http_content[] = "<html><head></head><body>Hello world!</body></html>";

static const char* TAG = "webserver.c";

static void webserver_process(struct netconn *conn) {
	ESP_LOGD(TAG, ">webserver_process")

	struct netbuf *netbuf;
	err_t err = netconn_recv(conn, &netbuf);
	if (err == ERR_OK) {

		char *buf;
		u16_t len;
		netbuf_data(netbuf, (void**) &buf, &len);

		// Request-Line = Method SP Request-URI SP HTTP-Version CRLF
		char *request_line = strtok(buf, "\n");
		if (request_line) {

			if (strstr(request_line, "GET / ")) {
				// the one and only response
				netconn_write(conn, http_ok, sizeof(http_ok) - 1, NETCONN_NOCOPY);
				netconn_write(conn, http_content, sizeof(http_content) - 1, NETCONN_NOCOPY);
			} else {
				ESP_LOGE(TAG, "Bad request: %s", request_line);
				netconn_write(conn, http_bad_request, sizeof(http_bad_request) - 1, NETCONN_NOCOPY);
			}
		} else {
			ESP_LOGE(TAG, "Bad request: expected newline")
			netconn_write(conn, http_bad_request, sizeof(http_bad_request) - 1, NETCONN_NOCOPY);
		}
	} else {
		ESP_LOGE(TAG, "netconn_recv error: %d", err)
		netconn_write(conn, http_server_error, sizeof(http_server_error) - 1, NETCONN_NOCOPY);
	}

	netconn_close(conn);
	netbuf_delete(netbuf);

	ESP_LOGD(TAG, "<webserver_process")
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
			webserver_process(newconn);
			netconn_delete(newconn);
		}
	} while (err == ERR_OK);

	netconn_close(conn);
	netconn_delete(conn);
	ESP_LOGE(TAG, "<webserver_task");
}
