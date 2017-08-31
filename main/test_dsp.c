// The author disclaims copyright to this source code.
#include "test_dsp.h"

#include <string.h>
#include "factory.h"
#include "hello_mp3.c"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "sdkconfig.h"

static const char* TAG = "test_dsp.c";

vs1053_handle_t test_dsp_handle;

/**
 * FreeRTOS DSP test task.
 */
void test_dsp_task(void *pvParameters) {
	ESP_LOGI(TAG, ">test_dsp_task");

	test_dsp_config_t *config = (test_dsp_config_t *)pvParameters;
	test_dsp_handle = config->vs1053_handle;

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

