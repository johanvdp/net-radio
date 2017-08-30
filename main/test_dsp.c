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

vs1053_handle_t test_dsp_handle;

void test_dsp_log_configuration() {
	ESP_LOGD(TAG, ">test_dsp_log_configuration");
	ESP_LOGI(TAG, "CONFIG_DSP_GPIO_XCS: %d", CONFIG_DSP_GPIO_XCS);
	ESP_LOGI(TAG, "CONFIG_DSP_GPIO_XDCS: %d", CONFIG_DSP_GPIO_XDCS);
	ESP_LOGI(TAG, "CONFIG_DSP_GPIO_DREQ: %d", CONFIG_DSP_GPIO_DREQ);
	ESP_LOGI(TAG, "CONFIG_DSP_SPI_SPEED_START_KHZ: %d", CONFIG_DSP_SPI_SPEED_START_KHZ);
	ESP_LOGI(TAG, "CONFIG_DSP_SPI_SPEED_KHZ: %d", CONFIG_DSP_SPI_SPEED_KHZ);
	ESP_LOGD(TAG, "<test_dsp_log_configuration");
}

/**
 * FreeRTOS MEM test task runs once.
 */
void test_dsp_task(void *ignore) {
	ESP_LOGI(TAG, ">test_dsp_task");

	test_dsp_log_configuration();

	vs1053_config_t configuration;
	memset(&configuration, 0, sizeof(vs1053_config_t));
	configuration.host = (spi_host_device_t) HSPI_HOST;
	configuration.clock_speed_start_hz = CONFIG_DSP_SPI_SPEED_START_KHZ * 1000;
	configuration.clock_speed_hz = CONFIG_DSP_SPI_SPEED_KHZ * 1000;
	configuration.xcs_io_num = CONFIG_DSP_GPIO_XCS;
	configuration.xdcs_io_num = CONFIG_DSP_GPIO_XDCS;
	configuration.dreq_io_num = CONFIG_DSP_GPIO_DREQ;
	configuration.rst_io_num = CONFIG_DSP_GPIO_RST;

	vs1053_begin(configuration, &test_dsp_handle);
	vs1053_set_volume(test_dsp_handle, 80, 80);

	while (1) {

		vs1053_decode_long(test_dsp_handle, (uint8_t*) &HELLO_MP3[0], sizeof(HELLO_MP3));

		vs1053_decode_end(test_dsp_handle);

		vTaskDelay(1000 / portTICK_PERIOD_MS);

		vs1053_soft_reset(test_dsp_handle);
	}

	// never reached
	//dsp_end(test_vs1053_handle);
	//ESP_LOGI(TAG, "<test_dsp_task");
	//vTaskDelete(NULL);
}

