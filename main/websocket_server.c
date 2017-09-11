// The author disclaims copyright to this source code.
#include "websocket_server.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "lwip/api.h"
#include "lwip/err.h"
#include "hwcrypto/sha.h"
#include "wpa2/utils/base64.h"
#include "sdkconfig.h"

static const char* TAG = "websocket_server";

/** HTTP response indicating switch to websocket protocol */
static const char HTTP_SWITCHING[] = "HTTP/1.1 101 Switching Protocols\r\n"
		"Upgrade: websocket\r\n"
		"Connection: Upgrade\r\n"
		"Sec-WebSocket-Accept: %.*s\r\n"
		"\r\n";
/** The length of the format string '%.*s' itself */
static const int HTTP_SWITCHING_FORMAT_LENGTH = 4;
/** HTTP header specifying client key */
static const char SEC_WEBSOCKET_KEY[] = "Sec-WebSocket-Key:";
/** WebSocket client key length. Can not find size in spec, example shown has length 24 */
static const int SEC_WEBSOCKET_KEY_LENGTH = 24;
/** WebSocket protocol GUID */
static const char SEC_WEBSOCKET_GUID[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
/** Number of bytes in SHA1 hash of client key plus GUID */
static const int SEC_WEBSOCKET_ACCEPT_SHA1_LENGTH = 20;
/** Number of mask bytes in frame (from client to server */
static const int FRAME_MASK_BYTES = 4;

/**
 * WebSocket frame opcode
 * https://tools.ietf.org/html/rfc6455#section-11.8
 *  |Opcode  | Meaning                             | Reference |
 */
typedef enum websocket_server_opcode_t {
	/**  | 0      | Continuation Frame                  | RFC 6455  | */
	CONTINUATION = 0,
	/**  | 1      | Text Frame                          | RFC 6455  | */
	TEXT = 1,
	/**  | 2      | Binary Frame                        | RFC 6455  | */
	BINARY = 2,
	/**  | 8      | Connection Close Frame              | RFC 6455  | */
	CONNECTION_CLOSE = 8,
	/**  | 9      | Ping Frame                          | RFC 6455  | */
	PING = 9,
	/**  | 10     | Pong Frame                          | RFC 6455  | */
	PONG = 10
} websocket_server_opcode_t;

/**
 * WebSocket frame format
 * https://developer.mozilla.org/en-US/docs/Web/API/WebSockets_API/Writing_WebSocket_servers#Format
 * ​​
 *       0                   1                   2                   3
 *       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *      +-+-+-+-+-------+-+-------------+-------------------------------+
 *      |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
 *      |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
 *      |N|V|V|V|       |S|             |   (if payload len==126/127)   |
 *      | |1|2|3|       |K|             |                               |
 *      +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
 *      |     Extended payload length continued, if payload len == 127  |
 *      + - - - - - - - - - - - - - - - +-------------------------------+
 *      |                               |Masking-key, if MASK set to 1  |
 *      +-------------------------------+-------------------------------+
 *      | Masking-key (continued)       |          Payload Data         |
 *      +-------------------------------- - - - - - - - - - - - - - - - +
 *      :                     Payload Data continued ...                :
 *      + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
 *      |                     Payload Data continued ...                |
 *      +---------------------------------------------------------------+
 */
typedef struct {
	struct netconn* connection;
	// frame header
	bool fin;
	websocket_server_opcode_t opcode;
	bool mask;
	// 64-bit extended payload length unsupported
	uint16_t payload_length;
	// frame payload
	char* payload;
} websocket_frame_t;

static uint16_t websocket_server_port;
static QueueHandle_t websocket_server_receive_queue;

/**
 * Create string representation of data bytes.
 * Useful for debugging exact byte contents
 */
static char* createHex(char *data, int length) {
	// space for string manipulation
	char *out = malloc(length * 2 + 1);
	assert(out != NULL);

	for (int i = 0; i < length; i++) {
		sprintf((out + i * 2), "%02X", data[i]);
	}
	*(out + length * 2) = 0;
	return out;
}

static char* createAcceptKey(char* request, unsigned int request_length) {
	// char *client_key = memmem(buf, i, SEC_WEBSOCKET_KEY, sizeof(SEC_WEBSOCKET_KEY));
	char *client_key = strstr(request, SEC_WEBSOCKET_KEY);
	if (client_key == NULL) {
		// not found
		return NULL;
	}
	unsigned char* sha_input = malloc(SEC_WEBSOCKET_KEY_LENGTH + sizeof(SEC_WEBSOCKET_GUID));
	assert(sha_input != NULL);
	unsigned char* sha_output = malloc(SEC_WEBSOCKET_ACCEPT_SHA1_LENGTH);
	assert(sha_output != NULL);
	// concatenate request key and websocket guid
	memcpy(sha_input, client_key + sizeof(SEC_WEBSOCKET_KEY), SEC_WEBSOCKET_KEY_LENGTH);
	memcpy(sha_input + SEC_WEBSOCKET_KEY_LENGTH, SEC_WEBSOCKET_GUID, sizeof(SEC_WEBSOCKET_GUID));
	unsigned int sha_input_length = SEC_WEBSOCKET_KEY_LENGTH + sizeof(SEC_WEBSOCKET_GUID);
	ESP_LOGD(TAG, "sha_input:%u %.*s", sha_input_length, sha_input_length, sha_input);
	// calculate sha1 hash
	esp_sha(SHA1, sha_input, strlen((char*) sha_input), sha_output);
	char* accept_key = (char*) _base64_encode(sha_output, SEC_WEBSOCKET_ACCEPT_SHA1_LENGTH, &request_length);
	free(sha_input);
	free(sha_output);
	return accept_key;
}

char* createSwitchingResponse(char* accept_key) {
	unsigned int accept_key_length = strlen(accept_key);
	// strip line ends (I see an extra LF on the wire)
	while (accept_key[accept_key_length - 1] <= 0x20) {
		accept_key_length--;
	}
	ESP_LOGD(TAG, "accept_key:%u %.*s", accept_key_length, accept_key_length, accept_key);

	char* response = malloc(sizeof(HTTP_SWITCHING) - HTTP_SWITCHING_FORMAT_LENGTH + accept_key_length);
	assert(response != NULL);
	sprintf(response, HTTP_SWITCHING, accept_key_length, accept_key);
	return response;
}

static void websocket_lifecycle(struct netconn *conn) {
	ESP_LOGD(TAG, ">websocket_lifecycle");

	struct netbuf *upgrade_netbuf;
	err_t err = netconn_recv(conn, &upgrade_netbuf);
	if (err != ERR_OK) {
		ESP_LOGE(TAG, "netconn_recv: %d", err);
	} else {
		char *request;
		u16_t request_length;
		netbuf_data(upgrade_netbuf, (void**) &request, &request_length);

		char* accept_key = createAcceptKey(request, request_length);
		if (accept_key == NULL) {
			ESP_LOGE(TAG, "HTTP header '%s' not found", SEC_WEBSOCKET_KEY);
		} else {
			char* response = createSwitchingResponse(accept_key);
			ESP_LOGD(TAG, "response: %.*s", strlen(response), response);
			netconn_write(conn, response, strlen(response), NETCONN_COPY);

			free(accept_key);
			free(response);

			struct netbuf *frame_netbuf;
			err_t err;
			while ((err = netconn_recv(conn, &frame_netbuf)) == ERR_OK) {

				// keep an eye on memory usage
				size_t available = heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT);
				ESP_LOGD(TAG, "heap_caps_get_minimum_free_size: %d", available);

				// read request
				netbuf_data(frame_netbuf, (void**) &request, &request_length);

				// build frame from request bytes
				int request_index = 0;
				websocket_frame_t frame;
				frame.fin = (request[request_index] & 0x80) > 0;
				frame.opcode = (request[request_index] & 0x0F);
				request_index++;
				frame.mask = (request[request_index] & 0x80) > 0;
				int payload_length = (request[request_index] & 0x7F);
				if (payload_length < 126) {
					frame.payload_length = payload_length;
				} else if (payload_length == 126) {
					request_index++;
					frame.payload_length = request[request_index] & 0xFF;
					request_index++;
					frame.payload_length <<= 8;
					frame.payload_length &= request[request_index] & 0xFF;
				} else if (payload_length == 127) {
					ESP_LOGE(TAG, "Unsupported payload length");
				}

				ESP_LOGD(TAG, "fin: %s", (frame.fin ? "true" : "false"));
				ESP_LOGD(TAG, "opcode: %u", frame.opcode);
				ESP_LOGD(TAG, "mask: %s", (frame.mask ? "true" : "false"));
				ESP_LOGD(TAG, "payload_length: %u", frame.payload_length);

				if (frame.opcode == CONNECTION_CLOSE) {
					// break from while loop receiving frames
					ESP_LOGD(TAG, "CONNECTION_CLOSE");
					break;
				}

				// allocate memory for unmasked payload
				char * payload = malloc(frame.payload_length);
				if (payload == NULL) {
					// should abort
					ESP_LOGE(TAG, "malloc failed");
				} else {
					// payload from client should be masked
					request_index++;
					char* start_of_mask = &request[request_index];
					if (frame.mask) {
						for (request_length = 0; request_length < frame.payload_length; request_length++) {
							payload[request_length] = (start_of_mask + FRAME_MASK_BYTES)[request_length]
									^ start_of_mask[request_length % FRAME_MASK_BYTES];
						}
					} else {
						ESP_LOGE(TAG, "payload should be masked");
						memcpy(payload, start_of_mask, frame.payload_length);
					}

					// ignore unsupported payload type
					if ((payload == NULL) || (frame.opcode != TEXT)) {
						ESP_LOGE(TAG, "unsupported payload");
					} else {
						frame.connection = conn;
						frame.payload = payload;

						// copies the frame, but this contains a reference to the payload that was just allocated
						// and must be free-ed by the receiving end
						xQueueSendFromISR(websocket_server_receive_queue, &frame, 0);
					}
				}
				netbuf_delete(frame_netbuf);
				frame_netbuf = NULL;
			}
			if (err != ERR_OK) {
				ESP_LOGE(TAG, "netconn_recv error:%d", err);
			}
			// if broken out of the loop, still delete the buffer
			if (frame_netbuf != NULL) {
				netbuf_delete(frame_netbuf);
			}
		}
		netbuf_delete(upgrade_netbuf);
	}

	ESP_LOGD(TAG, "<websocket_lifecycle");
}

static err_t write_text(websocket_frame_t frame) {
	ESP_LOGD(TAG, ">write_text");

	uint16_t payload_length = frame.payload_length;
	char* payload = frame.payload;
	struct netconn *connection = frame.connection;

	// create header
	unsigned char header[4];
	int header_length;
	// fin = true 0x80, rsv = not set 0x00, opcode = text 0x01
	header[0] = 0x81;
	if (payload_length < 126) {
		// mask = false 0x00, payload_length < 7 bits
		header[1] = (payload_length & 0x7F);
		header[2] = 0;
		header[3] = 0;
		header_length = 2;
		ESP_LOGD(TAG, "header: %02X %02X", header[0], header[1]);
	} else {
		// mask = false 0x00, payload_length > 7 bits
		header[1] = 126;
		// payload_length = 16 bits
		header[2] = (payload_length >> 8) & 0xFF;
		header[3] = payload_length & 0xFF;
		header_length = 4;
		ESP_LOGD(TAG, "header: %02X %02X %02X %02X", header[0], header[1], header[2], header[3]);
	}
	ESP_LOGD(TAG, "data: %u %.*s", payload_length, payload_length, payload);

	err_t err = netconn_write(connection, header, header_length, NETCONN_COPY | NETCONN_MORE);
	if (err == ERR_OK) {
		err = netconn_write(connection, payload, payload_length, NETCONN_COPY);
	}

	if (err != ERR_OK) {
		ESP_LOGE(TAG, "error: %d", err);
	}
	ESP_LOGD(TAG, "<write_text");
	return err;
}

void websocket_server_task(void *pvParameters) {
	ESP_LOGI(TAG, ">websocket_server_task");

	websocket_server_config_t *config = (websocket_server_config_t *) pvParameters;
	websocket_server_port = config->port;
	ESP_LOGD(TAG, "websocket_server_port: %u", websocket_server_port);

	err_t err;
	struct netconn *listening_conn = netconn_new(NETCONN_TCP);
	if (listening_conn == NULL) {
		ESP_LOGE(TAG, "netconn_new failed")
	} else {
		err = netconn_bind(listening_conn, NULL, websocket_server_port);
		if (err != ERR_OK) {
			ESP_LOGE(TAG, "netconn_bind error: %d", err)
		} else {
			err = netconn_listen(listening_conn);
			if (err != ERR_OK) {
				ESP_LOGE(TAG, "netconn_bind error: %d", err)
			} else {
				struct netconn *accepted_conn;
				do {
					err = netconn_accept(listening_conn, &accepted_conn);
					if (err != ERR_OK) {
						ESP_LOGE(TAG, "netconn_accept error: %d", err)
					} else {
						websocket_lifecycle(accepted_conn);
						netconn_close(accepted_conn);
						netconn_delete(accepted_conn);
					}
				} while (err == ERR_OK);
				netconn_close(listening_conn);
			}
		}
		netconn_delete(listening_conn);
	}
	ESP_LOGE(TAG, "<websocket_server_task");
}

void websocket_process_task(void *pvUnused) {
	ESP_LOGI(TAG, ">websocket_process_task");

	websocket_frame_t frame;

	websocket_server_receive_queue = xQueueCreate(10, sizeof(websocket_frame_t));

	while (1) {
		// get frame from queue
		if (xQueueReceive(websocket_server_receive_queue, &frame, 3 * portTICK_PERIOD_MS) == pdTRUE) {

			// echo frame to client (text, not masked, can be logged)
			ESP_LOGD(TAG, "process length: %u, data:%.*s", frame.payload_length, frame.payload_length, frame.payload);
			write_text(frame);

			// free payload in frame
			if (frame.payload != NULL) {
				free(frame.payload);
				frame.payload = NULL;
			}
		}
	}
}
