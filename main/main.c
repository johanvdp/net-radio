// The author disclaims copyright to this source code.
#include "main.h"

static const char* TAG = "main.c";

#define MAIN_BUFFER_BYTES 33

spi_device_handle_t vspi_bus_handle;
uint8_t *writeBuffer;
uint8_t *readBuffer;

void main_log_configuration() {
	ESP_LOGI(TAG, ">main_log_configuration");
	esp_chip_info_t chip_info;
	esp_chip_info(&chip_info);
	if (chip_info.model == CHIP_ESP32) {
		ESP_LOGI(TAG, "model: ESP32");
	} else {
		ESP_LOGI(TAG, "model: %d (unknown)", chip_info.model);
	}
	ESP_LOGI(TAG, "cores: %d", chip_info.cores);
	ESP_LOGI(TAG, "features WiFi: %s",
			((chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "YES" : "NO"));
	ESP_LOGI(TAG, "features BT: %s",
			((chip_info.features & CHIP_FEATURE_BT) ? "YES" : "NO"));
	ESP_LOGI(TAG, "features BLE: %s",
			((chip_info.features & CHIP_FEATURE_BLE) ? "YES" : "NO"));
	ESP_LOGI(TAG, "features flash: %s",
			((chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external"));
	ESP_LOGI(TAG, "flash size (MB): %d",
			(spi_flash_get_chip_size() / (1024 * 1024)));
	ESP_LOGI(TAG, "revision: %d", chip_info.revision);
	ESP_LOGI(TAG, "CONFIG_GPIO_HSPI_MISO: %d", CONFIG_GPIO_HSPI_MISO);
	ESP_LOGI(TAG, "CONFIG_GPIO_HSPI_MOSI: %d", CONFIG_GPIO_HSPI_MOSI);
	ESP_LOGI(TAG, "CONFIG_GPIO_HSPI_CLK: %d", CONFIG_GPIO_HSPI_CLK);
	ESP_LOGI(TAG, "CONFIG_GPIO_DSP_XDCS: %d", CONFIG_GPIO_DSP_XDCS);
	ESP_LOGI(TAG, "CONFIG_GPIO_DSP_XCS: %d", CONFIG_GPIO_DSP_XCS);
	ESP_LOGI(TAG, "CONFIG_GPIO_VSPI_MISO: %d", CONFIG_GPIO_VSPI_MISO);
	ESP_LOGI(TAG, "CONFIG_GPIO_VSPI_MOSI: %d", CONFIG_GPIO_VSPI_MOSI);
	ESP_LOGI(TAG, "CONFIG_GPIO_VSPI_CLK: %d", CONFIG_GPIO_VSPI_CLK);
	ESP_LOGI(TAG, "CONFIG_GPIO_I2C_SDA: %d", CONFIG_GPIO_I2C_SDA);
	ESP_LOGI(TAG, "CONFIG_GPIO_I2C_SCL: %d", CONFIG_GPIO_I2C_SCL);
	ESP_LOGI(TAG, "<main_log_configuration");
}

void main_vspi_initialize() {
	ESP_LOGI(TAG, ">main_vspi_initialize");
	spi_bus_config_t configuration;
	memset(&configuration, 0, sizeof(configuration));
	configuration.sclk_io_num = CONFIG_GPIO_VSPI_CLK;
	configuration.mosi_io_num = CONFIG_GPIO_VSPI_MOSI;
	configuration.miso_io_num = CONFIG_GPIO_VSPI_MISO;
	configuration.quadwp_io_num = -1;
	configuration.quadhd_io_num = -1;
	ESP_ERROR_CHECK(spi_bus_initialize(VSPI_HOST, &configuration, 1));
	ESP_LOGI(TAG, "<main_vspi_initialize");
}

void main_vspi_free() {
	ESP_LOGI(TAG, ">main_vspi_free");
	ESP_ERROR_CHECK(spi_bus_free(VSPI_HOST));
	ESP_LOGI(TAG, "<main_vspi_free");
}

// transfer per byte
void main_mem_byte() {
	ESP_LOGI(TAG, ">main_mem_byte");

	uint32_t address = 0;
	uint8_t w = 0;
	uint8_t r = 0;
	uint32_t errors = 0;
	for (address = 0; address < MEM_TOTAL_BYTES; address++) {
		// write
		mem_data_write_byte(address, w);
		// read
		r = mem_data_read_byte(address);
		// check
		if (r != w) {
			//ESP_LOGI(TAG, "%d w:%02x r:%02x", address, w, r);
			errors++;
		}
		w++;
	}
	ESP_LOGI(TAG, "errors: %d", errors);

	ESP_LOGI(TAG, "<main_mem_byte");
}

// transfer per page
void main_mem_page() {
	ESP_LOGI(TAG, ">main_mem_page");

	uint8_t r = 0;
	uint8_t w = 0;
	uint32_t address = 0;
	uint32_t index = 0;
	uint32_t errors = 0;

	for (address = 0; address < 32; address+=32) {
		// write
		w = 1;
		for (index = 0; index < MEM_BYTES_PER_PAGE; index++) {
			writeBuffer[index] = w;
			w++;
		}
		mem_data_write_page(address, writeBuffer);

		// read
		mem_data_read_page(address, readBuffer);

		// check
		errors = 0;
		for (index = 0; index < MEM_BYTES_PER_PAGE; index++) {
			r = readBuffer[index];
			w = writeBuffer[index];
			if (w != r) {
				ESP_LOGI(TAG, "%d w:%02x r:%02x", index, w, r);
				errors++;
			}
		}
	}
	ESP_LOGI(TAG, "errors: %d", errors);

	ESP_LOGI(TAG, "<main_mem_page");
}

void main_mem_mode() {
	mem_add_command((spi_host_device_t) VSPI_HOST);
	mem_command_write_mode_register(MEM_MODE_BYTE);
	uint8_t mode = mem_command_read_mode_register();
	ESP_LOGI(TAG, "mode: 0x%02x", mode);
	mem_command_write_mode_register(MEM_MODE_PAGE);
	mode = mem_command_read_mode_register();
	ESP_LOGI(TAG, "mode: 0x%02x", mode);
	mem_command_write_mode_register(MEM_MODE_SEQUENTIAL);
	mode = mem_command_read_mode_register();
	ESP_LOGI(TAG, "mode: 0x%02x", mode);
	mem_remove();
}

// transfer sequential
void main_mem_sequential() {
	ESP_LOGI(TAG, ">main_mem_sequential");

	uint8_t r = 0;
	uint8_t w = 0;
	uint32_t address = 0;
	uint32_t index = 0;
	uint32_t errors = 0;

	for (address = 0; address < MEM_TOTAL_BYTES; address+=MAIN_BUFFER_BYTES) {
		// write
		w = 1;
		for (index = 0; index < MAIN_BUFFER_BYTES; index++) {
			writeBuffer[index] = w;
			w++;
		}
		mem_data_write_page(address, writeBuffer);

		// read
		mem_data_read_page(address, readBuffer);

		// check
		errors = 0;
		for (index = 0; index < MAIN_BUFFER_BYTES; index++) {
			r = readBuffer[index];
			w = writeBuffer[index];
			if (w != r) {
				//ESP_LOGI(TAG, "%d w:%02x r:%02x", index, w, r);
				errors++;
			}
		}
	}
	ESP_LOGI(TAG, "errors: %d", errors);
	ESP_LOGI(TAG, "<main_mem_sequential");
}

void main_buffers_malloc() {
	ESP_LOGI(TAG, ">main_buffers_malloc");
	writeBuffer = mem_malloc(MAIN_BUFFER_BYTES);
	readBuffer = mem_malloc(MAIN_BUFFER_BYTES);
	ESP_LOGI(TAG, "<main_buffers_malloc");
}

void main_buffers_free() {
	mem_free(writeBuffer);
	mem_free(readBuffer);
}

void main_task(void *ignore) {
	ESP_LOGI(TAG, ">main_task");
	//esp_task_wdt_init();
	main_buffers_malloc();
	main_vspi_initialize();

	mem_add_command((spi_host_device_t) VSPI_HOST);
	mem_command_write_mode_register(MEM_MODE_PAGE);
	uint8_t mode = mem_command_read_mode_register();
	ESP_LOGI(TAG, "mode: 0x%02x", mode);
	mem_remove();

	mem_add_data((spi_host_device_t) VSPI_HOST);

	while (1) {
		//main_mem_mode();
		//main_mem_byte();
		main_mem_page();
		//main_mem_sequential();
	}

//	mem_free();
//	main_vspi_free();
//	main_buffers_free();
//	ESP_LOGI(TAG, "<main_task");
}

void app_main() {
	main_log_configuration();
	blink_log_configuration();
	mem_log_configuration();

	xTaskCreate(&main_task, "main_task", 4096, NULL, 5, NULL);
	xTaskCreate(&blink_task, "blink_task", 2048, NULL, 5, NULL);
}
