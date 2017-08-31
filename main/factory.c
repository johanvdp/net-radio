// The author disclaims copyright to this source code.
#include "factory.h"

#include <string.h>
//#include "freertos/FreeRTOS.h"
//#include "freertos/task.h"
//#include "esp_heap_caps.h"
//#include "esp_log.h"
//#include "driver/spi_master.h"
#include "sdkconfig.h"

static const char* TAG = "mem.c";

void factory_mem_log_configuration() {
	ESP_LOGD(TAG, ">factory_mem_log_configuration");
	ESP_LOGI(TAG, "CONFIG_MEM_GPIO_CS: %d", CONFIG_MEM_GPIO_CS);
	ESP_LOGI(TAG, "CONFIG_MEM_SPEED_MHZ: %d", CONFIG_MEM_SPEED_MHZ);
	ESP_LOGI(TAG, "CONFIG_MEM_TOTAL_BYTES: %d", CONFIG_MEM_TOTAL_BYTES);
	ESP_LOGI(TAG, "CONFIG_MEM_NUMBER_OF_PAGES: %d", CONFIG_MEM_NUMBER_OF_PAGES);
	ESP_LOGI(TAG, "CONFIG_MEM_BYTES_PER_PAGE: %d", CONFIG_MEM_BYTES_PER_PAGE);
	ESP_LOGD(TAG, "<factory_mem_log_configuration");
}

void factory_mem_create(spi_mem_handle_t *handle) {
	ESP_LOGD(TAG, ">factory_mem_create");

	factory_mem_log_configuration();

	spi_mem_config_t configuration;
	memset(&configuration, 0, sizeof(spi_mem_config_t));
	configuration.host = (spi_host_device_t) VSPI_HOST;
	configuration.clock_speed_hz = CONFIG_MEM_SPEED_MHZ * 1000000;
	configuration.spics_io_num = CONFIG_MEM_GPIO_CS;
	configuration.total_bytes = CONFIG_MEM_TOTAL_BYTES;
	configuration.number_of_pages = CONFIG_MEM_NUMBER_OF_PAGES;
	configuration.number_of_bytes_page = CONFIG_MEM_BYTES_PER_PAGE;

	spi_mem_log_config(configuration);
	spi_mem_begin(configuration, handle);
	spi_mem_log(*handle);

	ESP_LOGD(TAG, "<factory_mem_create");
}

void factory_dsp_log_configuration() {
	ESP_LOGD(TAG, ">factory_dsp_log_configuration");
	ESP_LOGI(TAG, "CONFIG_DSP_GPIO_XCS: %d", CONFIG_DSP_GPIO_XCS);
	ESP_LOGI(TAG, "CONFIG_DSP_GPIO_XDCS: %d", CONFIG_DSP_GPIO_XDCS);
	ESP_LOGI(TAG, "CONFIG_DSP_GPIO_DREQ: %d", CONFIG_DSP_GPIO_DREQ);
	ESP_LOGI(TAG, "CONFIG_DSP_SPI_SPEED_START_KHZ: %d", CONFIG_DSP_SPI_SPEED_START_KHZ);
	ESP_LOGI(TAG, "CONFIG_DSP_SPI_SPEED_KHZ: %d", CONFIG_DSP_SPI_SPEED_KHZ);
	ESP_LOGD(TAG, "<factory_dsp_log_configuration");
}

void factory_dsp_create(vs1053_handle_t *handle) {
	ESP_LOGD(TAG, ">factory_dsp_create");

	factory_dsp_log_configuration();

	vs1053_config_t configuration;
	memset(&configuration, 0, sizeof(vs1053_config_t));
	configuration.host = (spi_host_device_t) HSPI_HOST;
	configuration.clock_speed_start_hz = CONFIG_DSP_SPI_SPEED_START_KHZ * 1000;
	configuration.clock_speed_hz = CONFIG_DSP_SPI_SPEED_KHZ * 1000;
	configuration.xcs_io_num = CONFIG_DSP_GPIO_XCS;
	configuration.xdcs_io_num = CONFIG_DSP_GPIO_XDCS;
	configuration.dreq_io_num = CONFIG_DSP_GPIO_DREQ;
	configuration.rst_io_num = CONFIG_DSP_GPIO_RST;

	vs1053_log_config(configuration);
	vs1053_begin(configuration, handle);
	vs1053_log(*handle);

	ESP_LOGD(TAG, "<factory_dsp_create");
}

void factory_buffer_log_configuration() {
	ESP_LOGD(TAG, ">factory_buffer_log_configuration");
	//ESP_LOGI(TAG, "CONFIG_: %d", CONFIG_);
	ESP_LOGD(TAG, "<factory_buffer_log_configuration");
}

void factory_buffer_create(spi_mem_handle_t spi_mem_handle, uint32_t size, buffer_handle_t *handle) {
	ESP_LOGD(TAG, ">factory_buffer_create");

	factory_buffer_log_configuration();

	buffer_config_t configuration;
	configuration.spi_mem_handle = spi_mem_handle;
	configuration.size = size;

	buffer_log_config(configuration);
	buffer_begin(configuration, handle);
	buffer_log(*handle);

	ESP_LOGD(TAG, "<factory_buffer_create");
}
