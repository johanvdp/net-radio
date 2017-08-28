// The author disclaims copyright to this source code.
#include "blink.h"

static const char* TAG = "blink.c";

void blink_log_configuration() {
	ESP_LOGD(TAG, ">blink_log_configuration");
	ESP_LOGI(TAG, "CONFIG_BLINK_GPIO BLINK: %d", CONFIG_BLINK_GPIO);
	ESP_LOGI(TAG, "CONFIG_BLINK_ON_MS: %d", CONFIG_BLINK_ON_MS);
	ESP_LOGI(TAG, "CONFIG_BLINK_OFF_MS: %d", CONFIG_BLINK_OFF_MS);
	ESP_LOGD(TAG, "<blink_log_configuration");
}

void blink_task(void *pvUnused) {
	ESP_LOGD(TAG, ">blink_task");
	blink_log_configuration();

	// init
	gpio_pad_select_gpio(CONFIG_BLINK_GPIO);
	gpio_set_direction(CONFIG_BLINK_GPIO, GPIO_MODE_OUTPUT);

	// loop forever
	while (1) {
		/* OFF */
		gpio_set_level(CONFIG_BLINK_GPIO, 0);
		vTaskDelay(CONFIG_BLINK_OFF_MS / portTICK_PERIOD_MS);

		/* ON */
		gpio_set_level(CONFIG_BLINK_GPIO, 1);
		vTaskDelay(CONFIG_BLINK_ON_MS / portTICK_PERIOD_MS);
	}
	// never reached
	// ESP_LOGD(TAG, "<blink_task");
}

