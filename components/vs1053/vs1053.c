// The author disclaims copyright to this source code.
//
// Based on:
// - https://maniacbug.wordpress.com/2011/12/30/arduino-on-ice-internet-radio-via-shoutcast/
// - https://github.com/maniacbug/VS1053
// - http://www.vsdsp-forum.com/phpbb/viewtopic.php?f=11&t=65&p=308#p308
#include "vs1053.h"

#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include "esp_log.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

static const char* TAG = "dsp.c";

static const uint8_t VS1053_EMPTY_DATA[] = { //
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  //
		};

void vs1053_begin_control_start(vs1053_handle_t handle) {
	ESP_LOGD(TAG, ">vs1053_begin_control_start");
	spi_device_interface_config_t configuration;
	memset(&configuration, 0, sizeof(configuration));
	configuration.clock_speed_hz = handle->clock_speed_start_hz;
	configuration.spics_io_num = handle->xcs_io_num;
	configuration.queue_size = 1;
	ESP_ERROR_CHECK(spi_bus_add_device(handle->host, &configuration, &(handle->device_control)));
	ESP_LOGD(TAG, "<vs1053_begin_control_start");
}

void vs1053_begin_control(vs1053_handle_t handle) {
	ESP_LOGD(TAG, ">vs1053_begin_control");
	spi_device_interface_config_t configuration;
	memset(&configuration, 0, sizeof(configuration));
	configuration.clock_speed_hz = handle->clock_speed_hz;
	configuration.spics_io_num = handle->xcs_io_num;
	configuration.queue_size = 1;
	ESP_ERROR_CHECK(spi_bus_add_device(handle->host, &configuration, &(handle->device_control)));
	ESP_LOGD(TAG, "<vs1053_begin_control");
}

void vs1053_begin_data(vs1053_handle_t handle) {
	ESP_LOGD(TAG, ">vs1053_begin_data");
	spi_device_interface_config_t configuration;
	memset(&configuration, 0, sizeof(configuration));
	configuration.clock_speed_hz = handle->clock_speed_hz;
	configuration.spics_io_num = handle->xdcs_io_num;
	configuration.queue_size = 1;
	ESP_ERROR_CHECK(spi_bus_add_device(handle->host, &configuration, &(handle->device_data)));
	ESP_LOGD(TAG, "<vs1053_begin_data");
}

void vs1053_end_control(vs1053_handle_t handle) {
	ESP_LOGD(TAG, ">vs1053_end_control");
	ESP_ERROR_CHECK(spi_bus_remove_device(handle->device_control));
	handle->device_control = NULL;
	ESP_LOGD(TAG, "<vs1053_end_control");
}

void vs1053_end_data(vs1053_handle_t handle) {
	ESP_LOGD(TAG, ">vs1053_end_data");
	ESP_ERROR_CHECK(spi_bus_remove_device(handle->device_data));
	handle->device_data = NULL;
	ESP_LOGD(TAG, "<vs1053_end_data");
}

void vs1053_wait_dreq(vs1053_handle_t handle) {
	while (gpio_get_level(handle->dreq_io_num) == 0)
		;
}

void vs1053_write_register(vs1053_handle_t handle, uint8_t addressbyte, uint8_t highbyte, uint8_t lowbyte) {
	ESP_LOGD(TAG, ">vs1053_write_register 0x%02x 0x%02x%02x", addressbyte, highbyte, lowbyte);
	spi_transaction_t vs1053_spi_transaction;
	memset(&vs1053_spi_transaction, 0, sizeof(vs1053_spi_transaction));
	vs1053_spi_transaction.flags = SPI_TRANS_USE_TXDATA;
	vs1053_spi_transaction.length = 32;
	vs1053_spi_transaction.tx_data[0] = 0x02;
	vs1053_spi_transaction.tx_data[1] = addressbyte;
	vs1053_spi_transaction.tx_data[2] = highbyte;
	vs1053_spi_transaction.tx_data[3] = lowbyte;
	ESP_ERROR_CHECK(spi_device_transmit(handle->device_control, &vs1053_spi_transaction));

	// wait for dsp ready after command
	vs1053_wait_dreq(handle);

	ESP_LOGD(TAG, "<vs1053_write_register");
}

void vs1053_decode(vs1053_handle_t handle, uint8_t *data, uint8_t length) {
	ESP_ERROR_CHECK(length > VS1053_MAX_DATA_SIZE ? ESP_ERR_INVALID_SIZE : ESP_OK);
	// create transaction
	spi_transaction_t vs1053_spi_transaction;
	memset(&vs1053_spi_transaction, 0, sizeof(vs1053_spi_transaction));
	vs1053_spi_transaction.flags = 0;
	vs1053_spi_transaction.length = 8 * length;
	vs1053_spi_transaction.tx_buffer = data;
	// wait for ready for data
	vs1053_wait_dreq(handle);
	// transmit
	ESP_ERROR_CHECK(spi_device_transmit(handle->device_data, &vs1053_spi_transaction));
}

void vs1053_decode_long(vs1053_handle_t handle, uint8_t *data, uint16_t length) {
	ESP_LOGV(TAG, ">vs1053_decode_long");
	uint8_t *p = data;
	uint16_t remainder = length;
	while (remainder > 0) {
		// send maximum of 32 bytes
		uint8_t max = (remainder > VS1053_MAX_DATA_SIZE ? VS1053_MAX_DATA_SIZE : remainder);
		vs1053_decode(handle, p, max);
		p += max;
		remainder -= max;
	}
	ESP_LOGV(TAG, "<vs1053_decode_long");
}

void vs1053_decode_end(vs1053_handle_t handle) {
	ESP_LOGD(TAG, ">vs1053_decode_end");
	vs1053_decode(handle, (uint8_t *) &VS1053_EMPTY_DATA[0], sizeof(VS1053_EMPTY_DATA));
	ESP_LOGD(TAG, "<vs1053_decode_end");
}

void vs1053_set_volume(vs1053_handle_t handle, uint8_t left, uint8_t right) {
	vs1053_write_register(handle, VS1053_SCI_VOL, left, right);
}

void vs1053_wake(vs1053_handle_t handle) {
	ESP_LOGD(TAG, ">vs1053_wake");
	// Setting SCI_VOL to 0xFFFF will activate analog power down mode.
	vs1053_set_volume(handle, 0xFF, 0xFF);
	// Select slow sample rate (10Hz Mono)
	vs1053_write_register(handle, VS1053_SCI_AUDATA, 0, 10);
	// Switch on the analog parts
	vs1053_set_volume(handle, 0xFE, 0xFE);
	// Select low sample rate (8KHz Mono)
	vs1053_write_register(handle, VS1053_SCI_AUDATA, 31, 64);
	// Set initial volume (80 = -40dB)
	vs1053_set_volume(handle, 80, 80);
	ESP_LOGD(TAG, "<vs1053_wake");
}

void vs1053_soft_reset(vs1053_handle_t handle) {
	ESP_LOGD(TAG, ">vs1053_soft_reset");
	// Send reset
	vs1053_write_register(handle, VS1053_SCI_MODE, VS1053_SM_SDINEW, VS1053_SM_RESET);
	// Send clock register
	// 1011 0000 0000 0000
	// ---                 SC_MULT=XTALIx4.0 (clock multiplier)
	//    - -              SC_ADD=XTALIx2.0 (firmware allowance)
	//       --- ---- ---- SC_FREQ=0 (xtali=default 12.288MHz)
	vs1053_write_register(handle, VS1053_SCI_CLOCKF, 0xB0, 0x00);
	ESP_LOGD(TAG, "<vs1053_soft_reset");
}

void vs1053_hard_reset(vs1053_handle_t handle) {
	ESP_LOGD(TAG, ">vs1053_hard_reset");
	ESP_ERROR_CHECK(gpio_set_level(handle->rst_io_num, 0));
	vTaskDelay(10);
	ESP_ERROR_CHECK(gpio_set_level(handle->rst_io_num, 1));
	ESP_LOGD(TAG, "<vs1053_hard_reset");
}

void vs1053_log_config(vs1053_config_t config) {
	ESP_LOGD(TAG, ">vs1053_log_config");
	ESP_LOGI(TAG, "host: %d", config.host);
	ESP_LOGI(TAG, "clock_speed_start_hz: %d", config.clock_speed_start_hz);
	ESP_LOGI(TAG, "clock_speed_hz: %d", config.clock_speed_hz);
	ESP_LOGI(TAG, "xcs_io_num: %d", config.xcs_io_num);
	ESP_LOGI(TAG, "xdcs_io_num: %d", config.xdcs_io_num);
	ESP_LOGI(TAG, "dreq_io_num: %d", config.dreq_io_num);
	ESP_LOGI(TAG, "rst_io_num: %d", config.rst_io_num);
	ESP_LOGD(TAG, "<vs1053_log_config");
}

void vs1053_log(vs1053_handle_t handle) {
	ESP_LOGD(TAG, ">vs1053_log");
	ESP_LOGI(TAG, "handle: %p", handle);
	ESP_LOGI(TAG, "host: %d", handle->host);
	ESP_LOGI(TAG, "device_control: %p", handle->device_control);
	ESP_LOGI(TAG, "device_data: %p", handle->device_data);
	ESP_LOGI(TAG, "clock_speed_start_hz: %d", handle->clock_speed_start_hz);
	ESP_LOGI(TAG, "clock_speed_hz: %d", handle->clock_speed_hz);
	ESP_LOGI(TAG, "xcs_io_num: %d", handle->xcs_io_num);
	ESP_LOGI(TAG, "xdcs_io_num: %d", handle->xdcs_io_num);
	ESP_LOGI(TAG, "dreq_io_num: %d", handle->dreq_io_num);
	ESP_LOGI(TAG, "rst_io_num: %d", handle->rst_io_num);
	ESP_LOGD(TAG, "<vs1053_log");
}

void vs1053_begin(vs1053_config_t config, vs1053_handle_t *handle) {
	ESP_LOGD(TAG, ">vs1053_begin");

	// create a new handle
	vs1053_t *vs1053 = malloc(sizeof(vs1053_t));
	vs1053->host = config.host;
	vs1053->device_control = NULL;
	vs1053->device_data = NULL;
	vs1053->clock_speed_start_hz = config.clock_speed_start_hz;
	vs1053->clock_speed_hz = config.clock_speed_hz;
	vs1053->xcs_io_num = config.xcs_io_num;
	vs1053->xdcs_io_num = config.xdcs_io_num;
	vs1053->dreq_io_num = config.dreq_io_num;
	vs1053->rst_io_num = config.rst_io_num;

	gpio_pad_select_gpio(config.dreq_io_num);
	gpio_set_direction(config.dreq_io_num, GPIO_MODE_INPUT);

	gpio_pad_select_gpio(config.rst_io_num);
	ESP_ERROR_CHECK(gpio_set_direction(config.rst_io_num, GPIO_MODE_OUTPUT));
	ESP_ERROR_CHECK(gpio_set_level(config.rst_io_num, 1));

	vs1053_hard_reset(vs1053);

	// slow SPI
	vs1053_begin_control_start(vs1053);
	vs1053_soft_reset(vs1053);
	vs1053_wake(vs1053);
	vs1053_end_control(vs1053);

	// normal SPI
	vs1053_begin_control(vs1053);
	vs1053_begin_data(vs1053);

	*handle = vs1053;

	ESP_LOGD(TAG, "<vs1053_begin");
}

void vs1053_end(vs1053_handle_t handle) {
	ESP_LOGD(TAG, ">vs1053_end");
	vs1053_soft_reset(handle);
	vs1053_end_control(handle);
	vs1053_end_data(handle);
	free(handle);
	ESP_LOGD(TAG, "<vs1053_end");
}
