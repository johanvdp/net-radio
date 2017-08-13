// The author disclaims copyright to this source code.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

#define BLINK_GPIO 			CONFIG_GPIO_BLINK_LED
#define BLINK_ON_MS 		CONFIG_BLINK_ON_MS
#define BLINK_OFF_MS 		CONFIG_BLINK_OFF_MS
#define GPIO_HSPI_MISO 		CONFIG_GPIO_HSPI_MISO
#define GPIO_HSPI_MOSI 		CONFIG_GPIO_HSPI_MOSI
#define GPIO_HSPI_CLK  		CONFIG_GPIO_HSPI_CLK
#define GPIO_HSPI_RAM_CS	CONFIG_GPIO_HSPI_RAM_CS

static const char* TAG = "main.c";

void blink_task(void *pvParameter) {
	// init
	gpio_pad_select_gpio(BLINK_GPIO);
	gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

	// loop forever
	while (1) {
		/* OFF */
		gpio_set_level(BLINK_GPIO, 0);
		vTaskDelay(BLINK_OFF_MS / portTICK_PERIOD_MS);

		/* ON */
		gpio_set_level(BLINK_GPIO, 1);
		vTaskDelay(BLINK_ON_MS / portTICK_PERIOD_MS);
	}
}

void logCpuConfiguration() {
	esp_chip_info_t chip_info;
	esp_chip_info(&chip_info);
	ESP_LOGI(TAG, "ESP32, cores:%d, %s%s%s, revision: %d, flash: %dMB %s",
			chip_info.cores,
			((chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi" : ""),
			((chip_info.features & CHIP_FEATURE_BT) ? "/BT" : ""),
			((chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : ""),
			chip_info.revision, (spi_flash_get_chip_size() / (1024 * 1024)),
			((chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external"));
}

void logApplicationConfiguration() {
	ESP_LOGI(TAG, "BLINK_GPIO: %d", BLINK_GPIO);
	ESP_LOGI(TAG, "BLINK_ON_MS: %d", BLINK_ON_MS);
	ESP_LOGI(TAG, "BLINK_OFF_MS: %d", BLINK_OFF_MS);
}

void logConfiguration() {
	logCpuConfiguration();
	logApplicationConfiguration();
	fflush(stdout);
}

void spiConfigureBus() {
	ESP_LOGI(TAG, ">spiConfigureBus");
	spi_bus_config_t configuration;
	memset(&configuration, 0, sizeof(configuration));
	configuration.sclk_io_num = GPIO_HSPI_CLK;
	configuration.mosi_io_num = GPIO_HSPI_MOSI;
	configuration.miso_io_num = GPIO_HSPI_MISO;
	configuration.quadwp_io_num = -1;
	configuration.quadhd_io_num = -1;
	ESP_ERROR_CHECK(spi_bus_initialize(HSPI_HOST, &configuration, 0));
	ESP_LOGI(TAG, "<spiConfigureBus");
	fflush(stdout);
}

void spiConfigureDevice(spi_device_handle_t *handle) {
	ESP_LOGI(TAG, ">spiConfigureDevice");
	spi_device_interface_config_t configuration;
	memset(&configuration, 0, sizeof(configuration));
	//configuration.address_bits = 0;
	//configuration.command_bits = 0;
	//configuration.dummy_bits = 0;
	configuration.mode = 0;
	//configuration.duty_cycle_pos = 0;
	//configuration.cs_ena_posttrans = 0;
	//configuration.cs_ena_pretrans = 0;
	configuration.clock_speed_hz = 20000000;
	configuration.spics_io_num = GPIO_HSPI_RAM_CS;
	configuration.flags = 0;
	configuration.queue_size = 1;
	//configuration.pre_cb = NULL;
	//configuration.post_cb = NULL;
	ESP_ERROR_CHECK(spi_bus_add_device(HSPI_HOST, &configuration, handle));
	ESP_LOGI(TAG, "<spiConfigureDevice");
}

void spi_task(void *ignore) {
	ESP_LOGI(TAG, "spi_task");
	spiConfigureBus();

	spi_device_handle_t handle;
	memset(&handle, 0, sizeof(handle));
	spiConfigureDevice(&handle);

	uint8_t data[4];

	spi_transaction_t transaction;
	memset(&transaction, 0, sizeof(transaction));
	//transaction.address = 0;
	//transaction.command = 0;
	transaction.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
	transaction.length = 32;
	transaction.rxlength = 0;
	transaction.tx_data[0] = 0x55;
	transaction.tx_data[1] = 0xF0;
	transaction.tx_data[2] = 0x0F;
	transaction.tx_data[3] = 0xC3;
	//transaction.rx_buffer

	// observed:
	// period: 50us or 20kHz
	// clk: 50ns or 20MHz
	while (1) {
		ESP_ERROR_CHECK(spi_device_transmit(handle, &transaction));
	}
//  never reached code
//
//	ESP_LOGI(TAG, "spi_bus_remove_device");
//	ESP_ERROR_CHECK(spi_bus_remove_device(handle));
//
//	ESP_LOGI(TAG, "spi_bus_free");
//	ESP_ERROR_CHECK(spi_bus_free(HSPI_HOST));
}

void app_main() {
	logConfiguration();

	xTaskCreate(&blink_task, "blink_task", configMINIMAL_STACK_SIZE, NULL, 5,
			NULL);
	xTaskCreate(&spi_task, "spi_task", 8192, NULL, 5, NULL);
}
