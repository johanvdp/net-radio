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

void test_dsp_write(uint8_t *data, uint16_t length) {
	ESP_LOGD(TAG, ">test_dsp_write %p %d", data, length);
	uint8_t *p = data;
	uint16_t remainder = length;
	while (remainder > 0) {
		// send maximum of 32 bytes
		uint8_t max = (remainder > DSP_MAX_DATA_SIZE ? DSP_MAX_DATA_SIZE : remainder);
		dsp_decode(p, max);
		p += max;
		remainder -= max;
	}
	ESP_LOGD(TAG, "<test_dsp_write");
}

/**
 * FreeRTOS MEM test task runs once.
 */
void test_dsp_task(void *ignore) {
	ESP_LOGI(TAG, ">test_dsp_task");

	dsp_begin((spi_host_device_t) HSPI_HOST);
	dsp_set_volume(80, 80);

	while (1) {
		// send stream
		ESP_LOGD(TAG, "test_dsp_task send");
		test_dsp_write((uint8_t*)&HELLO_MP3[0], sizeof(HELLO_MP3));

		dsp_decode_end();

		vTaskDelay(1000 / portTICK_PERIOD_MS);

		dsp_soft_reset();
	}

	// never reached
	//ESP_LOGI(TAG, "<test_dsp_task");
	//vTaskDelete(NULL);
}

