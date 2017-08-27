// The author disclaims copyright to this source code.
//
// Based on:
// - https://maniacbug.wordpress.com/2011/12/30/arduino-on-ice-internet-radio-via-shoutcast/
// - https://github.com/maniacbug/VS1053
//
// - http://www.vsdsp-forum.com/phpbb/viewtopic.php?f=11&t=65&p=308#p308
#ifndef _DSP_H_
#define _DSP_H_

/**
 * @file
 * Driver to access VLSI VS1053b functions using the SPI bus.
 */

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "sdkconfig.h"
#include "esp_heap_caps.h"

/** Mode control, rw, 0x4000, 80 CLKI */
#define DSP_SCI_MODE        (0x00)
/** Status of VS1053b, rw, 0x000C, 80 CLKI */
#define DSP_SCI_STATUS      (0x01)
/** Built-in bass/treble control, rw, 0, 80 CLKI */
#define DSP_SCI_BASS        (0x02)
/** Clock freq + multiplier, rw, 0, 1200 XTALI */
#define DSP_SCI_CLOCKF      (0x03)
/** Decode time in seconds, rw, 0 , 100 CLKI */
#define DSP_SCI_DECODE_TIME (0x04)
/** Misc. audio data, rw, 0, 450 CLKI */
#define DSP_SCI_AUDATA      (0x05)
/** RAM write/read, rw, 0, 100 CLKI */
#define DSP_SCI_WRAM        (0x06)
/** Base address for RAM write/read, rw, 0, 100 CLKI */
#define DSP_SCI_WRAMADDR    (0x07)
/** Stream header data 0, r, 0, 80 CLKI */
#define DSP_SCI_HDAT0       (0x08)
/** Stream header data 1, r, 0, 80 CLKI */
#define DSP_SCI_HDAT1       (0x09)
/** Start address of application, rw, 0, 210 CLKI */
#define DSP_SCI_AIADDR      (0x0A)
/** Volume control, rw, 0, 80 CLKI*/
#define DSP_SCI_VOL         (0x0B)
/** Application control register 0, rw, 0, 80 CLKI */
#define DSP_SCI_AICTRL0     (0x0C)
/** Application control register 1, rw, 0, 80 CLKI */
#define DSP_SCI_AICTRL1     (0x0D)
/** Application control register 2, rw, 0, 80 CLKI */
#define DSP_SCI_AICTRL2     (0x0E)
/** Application control register 3, rw, 0, 80 CLKI */
#define DSP_SCI_AICTRL3     (0x0F)

//SCI_MODE is used to control the operation of VS1053b and defaults to 0x4800 (SM_SDINEW set).
/** Bit 0: Differential (0:normal in-phase audio,1:left channel inverted) */
#define DSP_SM_DIFF				(0x01)
/** Bit 1: Allow MPEG layers I & II (0:no, 1:yes) */
#define DSP_SM_LAYER12			(0x02)
/** Bit 2: Soft reset (0:no reset,1:reset) */
#define DSP_SM_RESET			(0x04)
/** Bit 3: Cancel decoding current file (0:no, 1:yes) */
#define DSP_SM_CANCEL			(0x08)
/** Bit 4: EarSpeaker low setting (0:off,1:active) */
#define DSP_SM_EARSPEAKER_LO	(0x10)
/** Bit 5: Allow SDI tests (0:not allowed,1:allowed) */
#define DSP_SM_TESTS			(0x20)
/** Bit 6: Stream mode (0:no, 1:yes) */
#define DSP_SM_STREAM			(0x40)
/** Bit 7: EarSpeaker high setting (0:off,1:active) */
#define DSP_SM_EARSPEAKER_HI	(0x80)
/** Bit 8: DCLK active edge (0:rising,1:falling) */
#define DSP_SM_DACT				(0x01)
/** Bit 9: SDI bit order (0:MSb first,1:MSb last) */
#define DSP_SM_SDIORD			(0x02)
/** Bit 10: Share SPI chip select (0:no, 1:yes) */
#define DSP_SM_SDISHARE			(0x04)
/** Bit 11: VS10xx native SPI modes (0:no, 1:yes) */
#define DSP_SM_SDINEW			(0x08)
/** Bit 12: PCM/ADPCM recording active (0:no, 1:yes) */
#define DSP_SM_ADPCM			(0x10)
/** Bit 13: undefined (0:right,1:wrong) */
#define DSP_SM_UNDEFINED		(0x20)
/** Bit 14: MIC / LINE1 selector (0:MICP,1:LINE1) */
#define DSP_SM_LINE1			(0x40)
/** Bit 15: Input clock range (0:12..13MHz,1:24..26MHz) */
#define DSP_SM_CLK_RANGE		(0x80)


/**
 * @brief Log configuration settings
 */
void dsp_log_configuration();

/**
 * @brief Initialize once on start
 */
void dsp_initialize(spi_host_device_t host);
void dsp_write_register(unsigned char addressbyte, unsigned char highbyte,
		unsigned char lowbyte);
void dsp_write_data(unsigned char byte);
void dsp_set_volume(unsigned char left, unsigned char right);
void dsp_wake();
void dsp_wait_dreq();
void dsp_soft_reset();

#endif
