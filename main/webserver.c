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

static const char* TAG = "webserver.c";

/**
 * The structure of a HTTP response:
 * Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF
 * One or more header lines CRLF
 * (An empty line) CRLF
 * Followed by the content (with a length as specified by the Content-Length header).
 */
static const char http_bad_request[] = "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\nConnection: Closed\r\n\r\n";
static const char http_server_error[] = "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\nConnection: Closed\r\n\r\n";
// the response contains one variable, the content length, and is cut in half
static const char http_ok_1[] = "HTTP/1.1 200 OKr\nContent-Type: text/html; charset=utf-8\r\nContent length: ";
static const char http_ok_2[] = "\r\nConnection: Closed\r\n\r\n";

// embed text file, see component.mk
extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[]   asm("_binary_index_html_end");
extern const uint8_t jquery_js_start[] asm("_binary_jquery_3_2_1_slim_min_js_start");
extern const uint8_t jquery_js_end[]   asm("_binary_jquery_3_2_1_slim_min_js_end");

static void webserver_write(struct netconn *conn, const void *begin, size_t length) {
	assert(length < 1000000);
	char content_length[6];
	int content_length_length = snprintf(content_length, 6, "%d", length);
	netconn_write(conn, http_ok_1, sizeof(http_ok_1) - 1, NETCONN_NOCOPY | NETCONN_MORE);
	netconn_write(conn, content_length, content_length_length, NETCONN_COPY | NETCONN_MORE);
	netconn_write(conn, http_ok_2, sizeof(http_ok_2) - 1, NETCONN_NOCOPY | NETCONN_MORE);
	netconn_write(conn, begin, length, NETCONN_NOCOPY);
}

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
				// index.html
				webserver_write(conn, index_html_start, index_html_end - index_html_start);
			} else if (strstr(request_line, "GET /jquery")) {
				// jquery.js
				webserver_write(conn, jquery_js_start, jquery_js_end - jquery_js_start);
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
