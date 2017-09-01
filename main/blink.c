// The author disclaims copyright to this source code.
#include "blink.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "driver/gpio.h"

static const char* TAG = "blink.c";

void blink_task(void *pvUnused) {
	ESP_LOGD(TAG, ">blink_task");
	ESP_LOGD(TAG, "CONFIG_BLINK_GPIO BLINK: %d", CONFIG_BLINK_GPIO);
	ESP_LOGD(TAG, "CONFIG_BLINK_ON_MS: %d", CONFIG_BLINK_ON_MS);
	ESP_LOGD(TAG, "CONFIG_BLINK_OFF_MS: %d", CONFIG_BLINK_OFF_MS);

	gpio_pad_select_gpio(CONFIG_BLINK_GPIO);
	gpio_set_direction(CONFIG_BLINK_GPIO, GPIO_MODE_OUTPUT);

	while (1) {
		/* OFF */
		gpio_set_level(CONFIG_BLINK_GPIO, 0);
		vTaskDelay(CONFIG_BLINK_OFF_MS / portTICK_PERIOD_MS);

		/* ON */
		gpio_set_level(CONFIG_BLINK_GPIO, 1);
		vTaskDelay(CONFIG_BLINK_ON_MS / portTICK_PERIOD_MS);
	}
	// should never be reached
}

