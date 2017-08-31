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
#include "esp_err.h"

static const char* TAG = "test_dsp.c";

vs1053_handle_t test_dsp_handle;

/**
 * DSP test task.
 */
esp_err_t test_dsp(test_dsp_config_t config) {
	ESP_LOGD(TAG, ">test_dsp");

	test_dsp_handle = config.vs1053_handle;
	ESP_LOGD(TAG, "test_dsp_handle: %p", test_dsp_handle);

	vs1053_decode_long(test_dsp_handle, (uint8_t*) &HELLO_MP3[0], sizeof(HELLO_MP3));
	vs1053_decode_end(test_dsp_handle);
	vTaskDelay(1000 / portTICK_PERIOD_MS);
	vs1053_soft_reset(test_dsp_handle);
	// todo verify dsp functioning correctly (status?)

	ESP_LOGD(TAG, "<test_dsp");
	return ESP_OK;
}

