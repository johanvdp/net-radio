// The author disclaims copyright to this source code.
#include "test_dsp.h"
#include "hello_mp3.c"

static const char* TAG = "test_dsp.c";

spi_device_handle_t vspi_bus_handle;

void test_dsp_log_configuration() {
	ESP_LOGD(TAG, ">test_dsp_log_configuration");
	// ESP_LOGI(TAG, "CONFIG_: %d", CONFIG_);
	ESP_LOGD(TAG, "<test_dsp_log_configuration");
}

/**
 * FreeRTOS MEM test task runs once.
 */
void test_dsp_task(void *ignore) {
	ESP_LOGI(TAG, ">test_dsp_task");

	dsp_initialize((spi_host_device_t) HSPI_HOST);
	dsp_set_volume(80,80);

	while (1) {
		// scan data
		ESP_LOGD(TAG, "dsp_initialize send");
		const unsigned char *p = &HELLO_MP3[0];
		const unsigned char *end = &HELLO_MP3[sizeof(HELLO_MP3) - 1];
		while (p <= end) {
			dsp_wait_dreq();
			// You can actually send 32 bytes here before checking for DREQ again
			dsp_write_data(*p++);
		}

		// send empty buffer
		ESP_LOGD(TAG, "dsp_initialize empty");
		for (unsigned int i = 0; i < 2048; i++) {
			dsp_wait_dreq();
			dsp_write_data(0);
		}
		dsp_soft_reset();
	}

	// never reached
	//ESP_LOGI(TAG, "<test_dsp_task");
	//vTaskDelete(NULL);
}

