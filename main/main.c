// The author disclaims copyright to this source code.
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

#define BLINK_GPIO CONFIG_GPIO_BLINK_LED
#define BLINK_ON_MS CONFIG_BLINK_ON_MS
#define BLINK_OFF_MS CONFIG_BLINK_OFF_MS

void blink_task(void *pvParameter)
{
	// init
    gpio_pad_select_gpio(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

    // loop forever
    while(1) {
        /* OFF */
        gpio_set_level(BLINK_GPIO, 0);
        vTaskDelay(BLINK_OFF_MS / portTICK_PERIOD_MS);

        /* ON */
        gpio_set_level(BLINK_GPIO, 1);
        vTaskDelay(BLINK_ON_MS / portTICK_PERIOD_MS);
    }
}

void app_main()
{
    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("ESP32 chip with %d CPU cores, %s%s%s, ",
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi" : "",
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");
    printf("silicon revision %d, ", chip_info.revision);
    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
    fflush(stdout);

    // start tasks
    xTaskCreate(&blink_task, "blink_task", configMINIMAL_STACK_SIZE, NULL, 5, NULL);
}
