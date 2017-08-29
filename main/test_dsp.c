// The author disclaims copyright to this source code.
#include "test_dsp.h"

#include <string.h>
#include "vs1053.h"
#include "hello_mp3.c"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "sdkconfig.h"

static const char* TAG = "test_dsp.c";

vs1053_handle_t test_vs1053_handle;

void test_dsp_log_configuration() {
	ESP_LOGD(TAG, ">test_dsp_log_configuration");
	ESP_LOGI(TAG, "CONFIG_DSP_GPIO_XCS: %d", CONFIG_DSP_GPIO_XCS);
	ESP_LOGI(TAG, "CONFIG_DSP_GPIO_XDCS: %d", CONFIG_DSP_GPIO_XDCS);
	ESP_LOGI(TAG, "CONFIG_DSP_GPIO_DREQ: %d", CONFIG_DSP_GPIO_DREQ);
	ESP_LOGI(TAG, "CONFIG_DSP_SPI_SPEED_START_KHZ: %d", CONFIG_DSP_SPI_SPEED_START_KHZ);
	ESP_LOGI(TAG, "CONFIG_DSP_SPI_SPEED_KHZ: %d", CONFIG_DSP_SPI_SPEED_KHZ);
	ESP_LOGD(TAG, "<test_dsp_log_configuration");
}

void test_dsp_write(uint8_t *data, uint16_t length) {
	ESP_LOGD(TAG, ">test_dsp_write %p %d", data, length);
	uint8_t *p = data;
	uint16_t remainder = length;
	while (remainder > 0) {
		// send maximum of 32 bytes
		uint8_t max = (remainder > VS1053_MAX_DATA_SIZE ? VS1053_MAX_DATA_SIZE : remainder);
		vs1053_decode(test_vs1053_handle, p, max);
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

	vs1053_config_t configuration;
	memset(&configuration, 0, sizeof(vs1053_config_t));
	configuration.host = (spi_host_device_t) HSPI_HOST;
	configuration.clock_speed_start_hz = CONFIG_DSP_SPI_SPEED_START_KHZ * 1000;
	configuration.clock_speed_hz = CONFIG_DSP_SPI_SPEED_KHZ * 1000;
	configuration.xcs_io_num = CONFIG_DSP_GPIO_XCS;
	configuration.xdcs_io_num = CONFIG_DSP_GPIO_XDCS;
	configuration.dreq_io_num = CONFIG_DSP_GPIO_DREQ;
	configuration.rst_io_num = CONFIG_DSP_GPIO_RST;

	vs1053_begin(configuration, &test_vs1053_handle);
	vs1053_set_volume(test_vs1053_handle, 80, 80);

	while (1) {
		// send stream
		ESP_LOGD(TAG, "test_dsp_task send");
		test_dsp_write((uint8_t*) &HELLO_MP3[0], sizeof(HELLO_MP3));

		vs1053_decode_end(test_vs1053_handle);

		vTaskDelay(1000 / portTICK_PERIOD_MS);

		vs1053_soft_reset(test_vs1053_handle);
	}

	// never reached
	//dsp_end(test_vs1053_handle);
	//ESP_LOGI(TAG, "<test_dsp_task");
	//vTaskDelete(NULL);
}

