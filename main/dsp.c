// The author disclaims copyright to this source code.
//
// Based on:
// - https://maniacbug.wordpress.com/2011/12/30/arduino-on-ice-internet-radio-via-shoutcast/
// - https://github.com/maniacbug/VS1053
// - http://www.vsdsp-forum.com/phpbb/viewtopic.php?f=11&t=65&p=308#p308
#include "dsp.h"

static const char* TAG = "dsp.c";

static const uint8_t DSP_EMPTY_DATA[] = {
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00  //
};

/** SPI clock speed startup [Hz] */
#define DSP_SPI_SPEED_START_HZ (CONFIG_DSP_SPI_SPEED_START_KHZ * 1000)
/** SPI clock speed [Hz] */
#define DSP_SPI_SPEED_HZ (CONFIG_DSP_SPI_SPEED_KHZ * 1000)

spi_host_device_t dsp_host;
spi_device_handle_t dsp_spi_control;
spi_device_handle_t dsp_spi_data;
spi_transaction_t dsp_spi_transaction;
uint8_t *dsp_buffer;

void dsp_log_configuration() {
	ESP_LOGD(TAG, ">dsp_log_configuration");
	ESP_LOGI(TAG, "CONFIG_DSP_GPIO_XCS: %d", CONFIG_DSP_GPIO_XCS);
	ESP_LOGI(TAG, "CONFIG_DSP_GPIO_XDCS: %d", CONFIG_DSP_GPIO_XDCS);
	ESP_LOGI(TAG, "CONFIG_DSP_GPIO_DREQ: %d", CONFIG_DSP_GPIO_DREQ);
	ESP_LOGI(TAG, "CONFIG_DSP_SPI_SPEED_START_KHZ: %d",
			CONFIG_DSP_SPI_SPEED_START_KHZ);
	ESP_LOGI(TAG, "CONFIG_DSP_SPI_SPEED_KHZ: %d", CONFIG_DSP_SPI_SPEED_KHZ);
	ESP_LOGD(TAG, "<dsp_log_configuration");
}


void dsp_buffer_malloc() {
	ESP_LOGD(TAG, ">dsp_buffer_malloc");
	dsp_buffer = heap_caps_malloc(DSP_MAX_DATA_SIZE, MALLOC_CAP_DMA);
	if (dsp_buffer == NULL) {
		ESP_LOGE(TAG, "heap_caps_malloc: out of memory");
		size_t available = heap_caps_get_minimum_free_size(MALLOC_CAP_DMA);
		ESP_LOGI(TAG, "heap_caps_get_minimum_free_size: %d", available);
	}
	memset(dsp_buffer, 0, DSP_MAX_DATA_SIZE);
	ESP_LOGD(TAG, "<dsp_buffer_malloc");
}

void dsp_buffer_free() {
	ESP_LOGD(TAG, ">dsp_buffer_free");
	heap_caps_free(dsp_buffer);
	ESP_LOGD(TAG, "<dsp_buffer_free");
}

void dsp_begin_control_start() {
	ESP_LOGD(TAG, ">dsp_begin_control_start");
	spi_device_interface_config_t configuration;
	memset(&configuration, 0, sizeof(configuration));
	configuration.clock_speed_hz = DSP_SPI_SPEED_START_HZ;
	configuration.spics_io_num = CONFIG_DSP_GPIO_XCS;
	configuration.queue_size = 1;
	ESP_ERROR_CHECK(
			spi_bus_add_device(dsp_host, &configuration, &dsp_spi_control));
	ESP_LOGD(TAG, "<dsp_begin_control_start");
}

void dsp_begin_control() {
	ESP_LOGD(TAG, ">dsp_begin_control");
	spi_device_interface_config_t configuration;
	memset(&configuration, 0, sizeof(configuration));
	configuration.clock_speed_hz = DSP_SPI_SPEED_HZ;
	configuration.spics_io_num = CONFIG_DSP_GPIO_XCS;
	configuration.queue_size = 1;
	ESP_ERROR_CHECK(
			spi_bus_add_device(dsp_host, &configuration, &dsp_spi_control));
	ESP_LOGD(TAG, "<dsp_begin_control");
}

void dsp_begin_data() {
	ESP_LOGD(TAG, ">dsp_begin_data");
	spi_device_interface_config_t configuration;
	memset(&configuration, 0, sizeof(configuration));
	configuration.clock_speed_hz = DSP_SPI_SPEED_HZ;
	configuration.spics_io_num = CONFIG_DSP_GPIO_XDCS;
	configuration.queue_size = 1;
	ESP_ERROR_CHECK(spi_bus_add_device(dsp_host, &configuration, &dsp_spi_data));
	ESP_LOGD(TAG, "<dsp_begin_data");
}

void dsp_end_control() {
	ESP_LOGD(TAG, ">dsp_end_control");
	ESP_ERROR_CHECK(spi_bus_remove_device(dsp_spi_control));
	ESP_LOGD(TAG, "<dsp_end_control");
}

void dsp_end_data() {
	ESP_LOGD(TAG, ">dsp_end_data");
	ESP_ERROR_CHECK(spi_bus_remove_device(dsp_spi_data));
	ESP_LOGD(TAG, "<dsp_end_data");
}

void dsp_wait_dreq() {
	while (gpio_get_level(CONFIG_DSP_GPIO_DREQ) == 0)
		;
}

void dsp_write_register(uint8_t addressbyte, uint8_t highbyte,
		uint8_t lowbyte) {
	ESP_LOGD(TAG, ">dsp_write_register 0x%02x 0x%02x%02x", addressbyte, highbyte, lowbyte);
	memset(&dsp_spi_transaction, 0, sizeof(dsp_spi_transaction));
	dsp_spi_transaction.flags = SPI_TRANS_USE_TXDATA;
	dsp_spi_transaction.length = 32;
	dsp_spi_transaction.tx_data[0] = 0x02;
	dsp_spi_transaction.tx_data[1] = addressbyte;
	dsp_spi_transaction.tx_data[2] = highbyte;
	dsp_spi_transaction.tx_data[3] = lowbyte;
	ESP_ERROR_CHECK(spi_device_transmit(dsp_spi_control, &dsp_spi_transaction));

	// wait for dsp ready after command
	dsp_wait_dreq();

	ESP_LOGD(TAG, "<dsp_write_register");
}

void dsp_decode(uint8_t *data, uint8_t length) {
	// copy to DMA capable region
	memcpy(dsp_buffer, data, length);
	// create transaction
	memset(&dsp_spi_transaction, 0, sizeof(dsp_spi_transaction));
	dsp_spi_transaction.flags = 0;
	dsp_spi_transaction.length = 8 * length;
	dsp_spi_transaction.tx_buffer = dsp_buffer;
	// wait for ready for data
	dsp_wait_dreq();
	// transmit
	ESP_ERROR_CHECK(spi_device_transmit(dsp_spi_data, &dsp_spi_transaction));
}

void dsp_decode_end() {
	ESP_LOGD(TAG, ">dsp_decode_end");
	dsp_decode((uint8_t *)&DSP_EMPTY_DATA[0], sizeof(DSP_EMPTY_DATA));
	ESP_LOGD(TAG, "<dsp_decode_end");
}

void dsp_set_volume(uint8_t left, uint8_t right) {
	dsp_write_register(DSP_SCI_VOL, left, right);
}

void dsp_wake() {
	ESP_LOGD(TAG, ">dsp_wake");
	// Setting SCI_VOL to 0xFFFF will activate analog power down mode.
	dsp_set_volume(0xFF, 0xFF);
	// Select slow sample rate (10Hz Mono)
	dsp_write_register(DSP_SCI_AUDATA, 0, 10);
	// Switch on the analog parts
	dsp_set_volume(0xFE, 0xFE);
	// Select low sample rate (8KHz Mono)
	dsp_write_register(DSP_SCI_AUDATA, 31, 64);
	// Set initial volume (40 = -20dB)
	dsp_set_volume(40, 40);
	ESP_LOGD(TAG, "<dsp_wake");
}

void dsp_soft_reset() {
	ESP_LOGD(TAG, ">dsp_soft_reset");
	// Send reset
	dsp_write_register(DSP_SCI_MODE, DSP_SM_SDINEW, DSP_SM_RESET);
	// Send clock register
	// 1011 0000 0000 0000
	// ---                 SC_MULT=XTALIx4.0 (clock multiplier)
	//    - -              SC_ADD=XTALIx2.0 (firmware allowance)
	//       --- ---- ---- SC_FREQ=0 (xtali=default 12.288MHz)
	dsp_write_register(DSP_SCI_CLOCKF, 0xB0, 0x00);
	ESP_LOGD(TAG, "<dsp_soft_reset");
}

void dsp_hard_reset() {
	ESP_LOGD(TAG, ">dsp_hard_reset");
	ESP_ERROR_CHECK(gpio_set_level(CONFIG_DSP_GPIO_RST, 0));
	vTaskDelay(10);
	ESP_ERROR_CHECK(gpio_set_level(CONFIG_DSP_GPIO_RST, 1));
	ESP_LOGD(TAG, "<dsp_hard_reset");
}

void dsp_begin(spi_host_device_t host) {
	ESP_LOGD(TAG, ">dsp_begin %d", host);

	dsp_host = host;

	gpio_pad_select_gpio(CONFIG_DSP_GPIO_DREQ);
	gpio_set_direction(CONFIG_DSP_GPIO_DREQ, GPIO_MODE_INPUT);

	gpio_pad_select_gpio(CONFIG_DSP_GPIO_RST);
	ESP_ERROR_CHECK(gpio_set_direction(CONFIG_DSP_GPIO_RST, GPIO_MODE_OUTPUT));
	ESP_ERROR_CHECK(gpio_set_level(CONFIG_DSP_GPIO_RST, 1));

	dsp_hard_reset();

	dsp_buffer_malloc();

	// slow SPI
	dsp_begin_control_start();
	dsp_soft_reset();
	dsp_wake();
	dsp_end_control();

	// normal SPI
	dsp_begin_control();
	dsp_begin_data();

	ESP_LOGD(TAG, "<dsp_begin");
}

void dsp_end() {
	ESP_LOGD(TAG, ">dsp_end");
	dsp_soft_reset();
	dsp_end_control();
	dsp_end_data();
	dsp_buffer_free();
	ESP_LOGD(TAG, "<dsp_end");
}
