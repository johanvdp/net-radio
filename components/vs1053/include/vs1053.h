// The author disclaims copyright to this source code.
//
// Based on:
// - https://maniacbug.wordpress.com/2011/12/30/arduino-on-ice-internet-radio-via-shoutcast/
// - https://github.com/maniacbug/VS1053
// - http://www.vsdsp-forum.com/phpbb/viewtopic.php?f=11&t=65&p=308#p308
#ifndef _VS1053_H_
#define _VS1053_H_

/**
 * @file
 * Driver to access VLSI VS1053b functions using the SPI bus.
 */

#include "driver/spi_master.h"

/** Mode control, rw, 0x4000, 80 CLKI */
#define VS1053_SCI_MODE        (0x00)
/** Status of VS1053b, rw, 0x000C, 80 CLKI */
#define VS1053_SCI_STATUS      (0x01)
/** Built-in bass/treble control, rw, 0, 80 CLKI */
#define VS1053_SCI_BASS        (0x02)
/** Clock freq + multiplier, rw, 0, 1200 XTALI */
#define VS1053_SCI_CLOCKF      (0x03)
/** Decode time in seconds, rw, 0 , 100 CLKI */
#define VS1053_SCI_DECODE_TIME (0x04)
/** Misc. audio data, rw, 0, 450 CLKI */
#define VS1053_SCI_AUDATA      (0x05)
/** RAM write/read, rw, 0, 100 CLKI */
#define VS1053_SCI_WRAM        (0x06)
/** Base address for RAM write/read, rw, 0, 100 CLKI */
#define VS1053_SCI_WRAMADDR    (0x07)
/** Stream header data 0, r, 0, 80 CLKI */
#define VS1053_SCI_HDAT0       (0x08)
/** Stream header data 1, r, 0, 80 CLKI */
#define VS1053_SCI_HDAT1       (0x09)
/** Start address of application, rw, 0, 210 CLKI */
#define VS1053_SCI_AIADDR      (0x0A)
/** Volume control, rw, 0, 80 CLKI*/
#define VS1053_SCI_VOL         (0x0B)
/** Application control register 0, rw, 0, 80 CLKI */
#define VS1053_SCI_AICTRL0     (0x0C)
/** Application control register 1, rw, 0, 80 CLKI */
#define VS1053_SCI_AICTRL1     (0x0D)
/** Application control register 2, rw, 0, 80 CLKI */
#define VS1053_SCI_AICTRL2     (0x0E)
/** Application control register 3, rw, 0, 80 CLKI */
#define VS1053_SCI_AICTRL3     (0x0F)

//SCI_MODE is used to control the operation of VS1053b and defaults to 0x4800 (SM_SDINEW set).
/** Bit 0: Differential (0:normal in-phase audio,1:left channel inverted) */
#define VS1053_SM_DIFF			s(0x01)
/** Bit 1: Allow MPEG layers I & II (0:no, 1:yes) */
#define VS1053_SM_LAYER12		(0x02)
/** Bit 2: Soft reset (0:no reset,1:reset) */
#define VS1053_SM_RESET			(0x04)
/** Bit 3: Cancel decoding current file (0:no, 1:yes) */
#define VS1053_SM_CANCEL		(0x08)
/** Bit 4: EarSpeaker low setting (0:off,1:active) */
#define VS1053_SM_EARSPEAKER_LO	(0x10)
/** Bit 5: Allow SDI tests (0:not allowed,1:allowed) */
#define VS1053_SM_TESTS			(0x20)
/** Bit 6: Stream mode (0:no, 1:yes) */
#define VS1053_SM_STREAM		(0x40)
/** Bit 7: EarSpeaker high setting (0:off,1:active) */
#define VS1053_SM_EARSPEAKER_HI	(0x80)
/** Bit 8: DCLK active edge (0:rising,1:falling) */
#define VS1053_SM_DACT			(0x01)
/** Bit 9: SDI bit order (0:MSb first,1:MSb last) */
#define VS1053_SM_SDIORD		(0x02)
/** Bit 10: Share SPI chip select (0:no, 1:yes) */
#define VS1053_SM_SDISHARE		(0x04)
/** Bit 11: VS10xx native SPI modes (0:no, 1:yes) */
#define VS1053_SM_SDINEW		(0x08)
/** Bit 12: PCM/ADPCM recording active (0:no, 1:yes) */
#define VS1053_SM_ADPCM			(0x10)
/** Bit 13: undefined (0:right,1:wrong) */
#define VS1053_SM_UNDEFINED		(0x20)
/** Bit 14: MIC / LINE1 selector (0:MICP,1:LINE1) */
#define VS1053_SM_LINE1			(0x40)
/** Bit 15: Input clock range (0:12..13MHz,1:24..26MHz) */
#define VS1053_SM_CLK_RANGE		(0x80)

/** Maximum data size accepted when DREQ active */
#define VS1053_MAX_DATA_SIZE (32)

typedef struct vs1053_config_t {
	spi_host_device_t host;
	int clock_speed_start_hz;
	int clock_speed_hz;
	int xcs_io_num;
	int xdcs_io_num;
	int dreq_io_num;
	int rst_io_num;
} vs1053_config_t;

/**
 * Even though this data is 'public'.
 * Do not shoot yourself in the foot by changing this data.
 */
typedef struct vs1053_t {
	spi_host_device_t host;
	spi_device_handle_t device_control;
	spi_device_handle_t device_data;
	int clock_speed_start_hz;
	int clock_speed_hz;
	int xcs_io_num;
	int xdcs_io_num;
	int dreq_io_num;
	int rst_io_num;
} vs1053_t;

typedef struct vs1053_t *vs1053_handle_t;

/**
 * @brief Begin using this component.
 */
void vs1053_begin(vs1053_config_t config, vs1053_handle_t *handle);
/**
 * @brief End using this component.
 */
void vs1053_end(vs1053_handle_t handle);
/**
 * @brief Decode stream. Allowing only short data chunks.
 * @param handle Component handle.
 * @param data The data to send.
 * @param length Length of the data to send. Obey VS1053 maximum data size.
 */
void vs1053_decode(vs1053_handle_t handle, uint8_t *data, uint8_t length);
/**
 * @brief Decode stream.
 * @param handle Component handle.
 * @param data The data to send.
 * @param length Length of the data to send.
 */
void vs1053_decode_long(vs1053_handle_t handle, uint8_t *data, uint16_t length);
/**
 * @brief End stream. Sends an empty data chunk to the decoder.
 * @param handle Component handle.
 */
void vs1053_decode_end(vs1053_handle_t handle);
/**
 * @brief Set volume.
 * The channel volume sets the attenuation from the maximum volume level in 0.5 dB steps.
 * Thus, maximum volume is 0x0000 and total silence is 0xFEFE.
 * Setting SCI_VOL to 0xFFFF will activate analog powerdown mode.
 * Note, that after hardware reset the volume is set to full volume.
 * Resetting the software does not reset the volume setting.
 * @param handle Component handle.
 * @param left Left channel volume.
 * @param right Right channel volume.
 */
void vs1053_set_volume(vs1053_handle_t handle, uint8_t left, uint8_t right);
void vs1053_wake(vs1053_handle_t handle);
void vs1053_soft_reset(vs1053_handle_t handle);

#endif
