// The author disclaims copyright to this source code.
#include "blink.h"

static const char* TAG = "blink.c";

void blink_log_configuration() {
	ESP_LOGI(TAG, ">blink_log_configuration");
	ESP_LOGI(TAG, "CONFIG_GPIO BLINK: %d", CONFIG_GPIO_BLINK);
	ESP_LOGI(TAG, "CONFIG_BLINK_ON_MS: %d", CONFIG_BLINK_ON_MS);
	ESP_LOGI(TAG, "CONFIG_BLINK_OFF_MS: %d", CONFIG_BLINK_OFF_MS);
	ESP_LOGI(TAG, "<blink_log_configuration");
}

void blink_task(void *pvUnused) {
	ESP_LOGI(TAG, ">blink_task");

	// init
	gpio_pad_select_gpio(CONFIG_GPIO_BLINK);
	gpio_set_direction(CONFIG_GPIO_BLINK, GPIO_MODE_OUTPUT);
	esp_task_wdt_init();

	// loop forever
	while (1) {
		/* OFF */
		gpio_set_level(CONFIG_GPIO_BLINK, 0);
		esp_task_wdt_feed();
		vTaskDelay(CONFIG_BLINK_OFF_MS / portTICK_PERIOD_MS);

		/* ON */
		gpio_set_level(CONFIG_GPIO_BLINK, 1);
		esp_task_wdt_feed();
		vTaskDelay(CONFIG_BLINK_ON_MS / portTICK_PERIOD_MS);
	}
}

