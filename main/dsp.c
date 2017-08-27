// The author disclaims copyright to this source code.
//
// Based on:
// - https://maniacbug.wordpress.com/2011/12/30/arduino-on-ice-internet-radio-via-shoutcast/
// - https://github.com/maniacbug/VS1053
// - http://www.vsdsp-forum.com/phpbb/viewtopic.php?f=11&t=65&p=308#p308
#include "dsp.h"

static const char* TAG = "dsp.c";

/** SPI clock speed startup [Hz] */
#define DSP_SPI_SPEED_START_HZ (CONFIG_DSP_SPI_SPEED_START_KHZ * 1000)
/** SPI clock speed [Hz] */
#define DSP_SPI_SPEED_HZ (CONFIG_DSP_SPI_SPEED_KHZ * 1000)

spi_host_device_t dsp_host;
spi_device_handle_t dsp_spi_control;
spi_device_handle_t dsp_spi_data;
spi_transaction_t dsp_spi_transaction;

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
	ESP_LOGD(TAG, "<dsp_write_register");
}

void dsp_write_data(uint8_t byte) {
	memset(&dsp_spi_transaction, 0, sizeof(dsp_spi_transaction));
	dsp_spi_transaction.flags = SPI_TRANS_USE_TXDATA;
	dsp_spi_transaction.length = 8;
	dsp_spi_transaction.tx_data[0] = byte;
	ESP_ERROR_CHECK(spi_device_transmit(dsp_spi_data, &dsp_spi_transaction));
}

void dsp_set_volume(uint8_t left, uint8_t right) {
	dsp_write_register(DSP_SCI_VOL, left, right);
}

// wait clki
// apply magic with CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ and CONFIG_DSP_CLKI later
static inline void dsp_wait_clki(int cycles) {
	vTaskDelay(1);
//	for (int i = 0; i < cycles; i++) {
//		__asm__ __volatile__("nop");
//	}
}

// wait xtali cycles
// apply magic with CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ and CONFIG_DSP_XTALI later
static inline void dsp_wait_xtali(int cycles) {
	vTaskDelay(1);
//	for (int i = 0; i < cycles; i++) {
//		__asm__ __volatile__("nop");
//	}
}

void dsp_wake() {
	// Setting SCI_VOL to 0xFFFF will activate analog powerdown mode.
	dsp_set_volume(0xFF, 0xFF);
	// Select slow sample rate for slow analog part startup (10Hz Mono)
	dsp_write_register(DSP_SCI_AUDATA, 0, 10);
	dsp_wait_clki(450);

	// Switch on the analog parts
	dsp_set_volume(0xFE, 0xFE);
	// Select low sample rate (8KHz Mono)
	dsp_write_register(DSP_SCI_AUDATA, 31, 64);
	// Set initial volume (40 = -20dB)
	dsp_set_volume(40, 40);
}

void dsp_wait_dreq() {
	while (gpio_get_level(CONFIG_DSP_GPIO_DREQ) == 0)
		;
}

void dsp_soft_reset() {
	ESP_LOGD(TAG, ">dsp_soft_reset");
	dsp_write_register(DSP_SCI_MODE, DSP_SM_SDINEW, DSP_SM_RESET);
	dsp_wait_clki(80);
	dsp_wait_dreq();
	// Set clock register, doubler etc.
	// 1011 0000 0000 0000
	// ---                 SC_MULT=XTALIx4.0 (clock multiplier)
	//    - -              SC_ADD=XTALIx2.0 (firmware allowance)
	//       --- ---- ---- SC_FREQ=0 (xtali=default 12.288MHz)
	dsp_write_register(DSP_SCI_CLOCKF, 0xB0, 0x00);
	dsp_wait_xtali(1200);
	dsp_wait_dreq();
	ESP_LOGD(TAG, "<dsp_soft_reset");
}

void dsp_initialize(spi_host_device_t host) {
	ESP_LOGD(TAG, ">dsp_initialize %d", host);

	dsp_host = host;

	gpio_pad_select_gpio(CONFIG_DSP_GPIO_DREQ);
	gpio_set_direction(CONFIG_DSP_GPIO_DREQ, GPIO_MODE_INPUT);

	gpio_pad_select_gpio(CONFIG_DSP_GPIO_RST);
	ESP_ERROR_CHECK(gpio_set_direction(CONFIG_DSP_GPIO_RST, GPIO_MODE_OUTPUT));
	ESP_ERROR_CHECK(gpio_set_level(CONFIG_DSP_GPIO_RST, 1));
	ESP_ERROR_CHECK(gpio_set_level(CONFIG_DSP_GPIO_RST, 0));
	vTaskDelay(100);
	ESP_ERROR_CHECK(gpio_set_level(CONFIG_DSP_GPIO_RST, 1));

	// slow SPI
	dsp_begin_control_start();
	dsp_soft_reset();
	dsp_wake();
	dsp_end_control();

	// normal SPI
	dsp_begin_control();
	dsp_begin_data();

	ESP_LOGD(TAG, "<dsp_initialize");
}

