#Internet radio
The internet radio is used to play audio from radio station streams (http). Details such as the network location are configured (externally) and accessible to the user as favorites.

[TOC]

# Use cases

## Resume
After power on, or when the audio is unmuted, the radio will resume playing depending on information available in the system. In order of preference:

+ Resume playing the most recent favorite, on the most recent volume.
+ Start playing the first favorite, on low volume.
+ Do nothing.

# User interface

## Navigation keys
The keys are typically used to switch favorites while playing, or to navigate the menu. (The first version will have only one page, and no menu). The function of the keys is context specific. Information on the display includes symbols to indicate which keys are applicable.

**Table: Key functions**

|Key|Function|
|:-:|:-------|
|↑  |Up      |
|↓  |Down    |
|↖  |Home    |
|←  |Left    |
|→  |Right   |

**Table: Key layout**

| | | |
|-|-|-|
| |↑| |
|←|↖|→|
| |↓| |

## Volume knob
The volume knob is used to change the volume of the audio playing.

**Table: Volumne knob**

|Action|Function                             |
|:----:|:------------------------------------|
|↻     |Rotate clockwise, Volume up          |
|↺     |Rotate counter clockwise, Volume down|
|↧     |Mute, Unmute                         |

## Display
The display can show different pages. Only one page is displayed at a time. The page can, for example, be showing details of the current favorite playing, or show a list of options. The page content can include symbols to indicate which keys can be used.

**Table: Display symbols**

|Symbol|Description   |
|:----:|:-------------|
|↖     |Home          |
|↑     |Up            |
|↓     |Down          |
|↕     |Up and down   |
|←     |Left          |
|→     |Right         |
|↔     |Left and right|

### Pages

#### Home
Display stream details

**Table: Home layout**

||        |
||--------|
||Favorite|
||Title   |
||Artist  |
||Album   |

**Table: Home keys**

|Key|Function                             |
|:-:|:------------------------------------|
|↑  |Previous favorite                    |
|↓  |Next favorite                        |
|↖  |Home                                 |
|←  |Left                                 |
|→  |Right                                |
|↻  |Rotate clockwise, volume up          |
|↺  |Rotate counter clockwise, volume down|
|↧  |Mute                                 |

# Software

## UserInput
+ Read IO
+ Evaluate button state
	+ Debounce
+ Provide button state
	+ Button
		+ Is pressed
		+ Duration
	+ Encoder
		+ Relative movement

## Display
+ Provide frame buffer
+ Update LCD

## Page
+ Evaluate user input
+ Perform action
+ Update display

## Reader
	+ Read file
		+ Read data from network file and write to buffer
	+ Read stream
		+ Read data from network stream and write to buffer

## Buffer
	+ Provide read and write methods

## Player
	+ Read from buffer (stream source)
	+ Send to DSP (stream sink)

## Boundary
	+ Provides debug interface
	+ Interpret control commands

## I2C
	+ Arbitrate usage
		+ IO
		+ LCD

## SPI
	+ Arbitrate usage
		+ DSP
		+ MEM

## Settings
+ Favorites
	+ Favorite
		+ scheme, authority, path, query, fragment
		(http://example.com:8042/over/there?name:ferret#nose)
+ Network credentials
	+ Station identifier
	+ Username and password

## Volatile
+ Current page
+ Currently favorite
+ Current volume
+ Current state
	+ stopped
	+ playing

# Hardware
## Modules
### PSU power supply
110-240VAC to 12VDC 3A module.

**Table: PSU terminals**

|Group |Terminal|Function|
|:-----|:-------|:-------|
|AC    |L       |230V L  |
|AC    |N       |230V N  |
|Power |12V     |12V     |
|Power |GND     |GND     |

### VRM voltage regulator
LM2596 3A step-down switching regulator

+ 12V input
+ 5V output

**Table: VRM terminals**

|Group |Terminal|Function|
|:-----|:-------|:-------|
|Power |IN      |12V     |
|Power |GND     |GND     |
|Power |OUT     |5V      |
|Power |GND     |GND     |

### CPU processor
WeMos D1 mini pro module.

+ 16M bytes(128M bit) flash memory
+ External antenna connector
+ Built-in ceramic antenna
+ CP2104 USB to serial controller

**Table: CPU terminals**

|Group  |Terminal|Function                                                        |
|:------|:-------|:---------------------------------------------------------------|
|       |RST     |RST input (active low), not connected                           |
|       |A0      |ADC TOUT (unused), not connected                                |
|Control|D0      |GPIO16 WAKE, DSP.DREQ                                           |
|SPI    |D5      |GPIO14 HSPI_CLK, CLK                                            |
|SPI    |D6      |GPIO12 HSPI_Q, MISO                                             |
|SPI    |D7      |GPIO13 HSPI_D, MOSI                                             |
|Control|D8      |GPIO15 10k pulldown, boot mode, MEM.CS (active low)             |
|Power  |3V3     |not connected                                                   |
|Serial |TX      |GPIO1 TXD0, not connected                                       |
|Serial |RX      |GPIO3 RXD0, not connected                                       |
|I2C    |D1      |GPIO5 SCL                                                       |
|I2C    |D2      |GPIO4 SDA                                                       |
|Control|D3      |GPIO0 10k pull-up, boot mode, DSP.XDCS (active low)             |
|Control|D4      |GPIO2 10k pull-up, built in led, boot mode, DSP.XCS (active low)|
|Power  |G       |GND                                                             |
|Power  |5V      |5V                                                              |

### LVL digital level converter
BSS138 based digital level converter module.

**Table: terminals**

|Group  |Terminal|Function     |
|:------|:-------|:------------|
|Power  |HV      |5V           |
|Power  |LV      |3V3          |
|Power  |GND     |GND          |
|I2C    |HV1     |SDA_H        |
|I2C    |LV1     |SDA_L        |
|I2C    |HV2     |SCL_H        |
|I2C    |LV2     |SCL_L        |
|Control|HV3     |RST_H        |
|Control|LV3     |RST_L        |
|       |HV4     |not connected|
|       |LV4     |not connected|

### DSP audio decoder
AlienTek VS1053 audio decoder module.

**Table: DSP terminals**

|Group  |Terminal     |Function                                                     |
|:------|:------------|:------------------------------------------------------------|
|Control|P1.1         |RST reset input (active low)                                 |
|Control|P1.2         |DREQ data request output (buffer can take 32 bytes when high)|
|SPI    |P1.3         |SO serial output (active when XCS low)                       |
|SPI    |P1.4         |SI serial input (sampled on rising edge SCK)                 |
|SPI    |P1.5         |SCK serial clock input (can be continuous or gated)          |
|Control|P1.6         |XDCS data interface chip select (active low)                 |
|Control|P1.7         |XCS control interface chip select (active low)               |
|Power  |P1.8         |3V3 output (500mA max)                                       |
|Power  |P1.9         |5V input                                                     |
|Power  |P1.10        |GND                                                          |
|       |P2           |not connected                                                |
|       |LINE_IN      |not connected                                                |
|Audio  |PHONE.GND    |GND                                                          |
|Audio  |PHONE.LEFT   |LEFT                                                         |
|Audio  |PHONE.RIGHT  |RIGHT                                                        |

### IO I/O
MCP23017 16-bit I/O expander module.

+ I2C address 0x21

**Table: IO terminals**

|Group  |Terminal|Function                 |
|:------|:-------|:------------------------|
|Power  |VCC     |5V                       |
|Power  |GND     |GND                      |
|I2C    |SDA     |SDA                      |
|I2C    |SCL     |SCL                      |
|Control|RESET   |5V                       |
|       |A0      |5V                       |
|       |A1      |GND                      |
|       |A2      |GND                      |
|Keys   |GPA0    |KEY_UP input             |
|Keys   |GPA1    |KEY_DOWN input           |
|Keys   |GPA2    |KEY_LEFT input           |
|Keys   |GPA3    |KEY_RIGHT input          |
|Keys   |GPA4    |KEY_HOME input           |
|Knob   |GPA5    |ENCODER_A input          |
|Knob   |GPA6    |ENCODER_B input          |
|Knob   |GPA7    |ENCODER_SWITCH           |
|Control|GPB0    |MUTE (active low) output |
|Control|GPB1    |RESET (active low) output|

### LCD display
128x64 pixel LCD display module controlled via an MCP23017 16-bit I/O expander.

+ I2C address 0x20

**Table: LCD terminals**

|Group|Terminal|Function|
|:----|:-------|:-------|
|Power|VCC     |5V      |
|Power|GND     |GND     |
|I2C  |SDA     |SDA     |
|I2C  |SCL     |SCL     |

### MEM nemory
23LC1024 1Mbit SPI SRAM module.

**Table: MEM terminals**

|Group  |Terminal|Function               |
|:------|:-------|:----------------------|
|Control|1       |CS (active low)        |
|SPI    |2       |SO                     |
|       |3       |not connected          |
|Power  |4       |GND                    |
|SPI    |5       |SI                     |
|SPI    |6       |SCK                    |
|       |7       |HOLD (active low) = 3V3|
|Power  |8       |VCC = 3V3              |

### AMP amplifier
TPA3110 2x15W class D audio amplifier module.

**Table: AMP terminals**

|Group  |Terminal |Function          |
|:------|:--------|:-----------------|
|Power  |POWER.1  |12V               |
|Power  |POWER.2  |GND               |
|Control|MUTE.1   |MUTE (active low) |
|       |MUTE.2   |GND, not connected|
|Audio  |LINE_IN.1|LEFT              |
|Audio  |LINE_IN.2|GND               |
|Audio  |LINE_IN.3|RIGHT             |
|Audio  |LEFT.1   |LEFT+             |
|Audio  |LEFT.2   |LEFT-             |
|Audio  |RIGHT.1  |RIGHT+            |
|Audio  |RIGHT.2  |RIGHT-            |

### AUX terminals
2.54 mm pitch screw terminal block.

**Table: AUX terminals**

|Group|Terminal|Function      |
|:----|:-------|:-------------|
|Power|1       |12V           |
|Power|2       |GND           |
|Audio|3       |LEFT-         |
|Audio|4       |LEFT+         |
|Audio|5       |RIGHT-        |
|Audio|6       |RIGHT+        |
|User |7       |GND           |
|User |8       |KEY_UP        |
|User |9       |KEY_DOWN      |
|User |10      |KEY_LEFT      |
|User |11      |KEY_RIGHT     |
|User |12      |KEY_HOME      |
|User |13      |GND           |
|User |14      |ENCODER_A     |
|User |15      |ENCODER_B     |
|User |16      |ENCODER_SWITCH|

## Connections

### Power
**Table: Power connections**

|Function|PSU|VRM|CPU|LVL|IO |LCD|DSP  |AMP    |MEM|
|:-------|:--|:--|:--|:--|:--|:--|:----|:------|:--|
|GND     |GND|GND|G  |GND|GND|GND|P1.10|Power.2|4  |
|12V     |12V|IN |   |   |   |   |     |Power.1|   |
|5V      |   |OUT|5V |HV |VCC|VCC|P1.9 |       |   |
|3V3     |   |   |   |LV |   |   |3V3  |       |8  |

### I2C
**Table: I2C connections**

|Function|CPU|LVL|IO |LCD|
|:-------|:--|:--|:--|:--|
|CPU.SDA |D2 |LV1|   |   |
|LVL.SDA |   |HV1|SDA|SDA|
|CPU.SCL |D1 |LV2|   |   |
|LVL.SCL |   |HV2|SCL|SCL|

### SPI
**Table: SPI connections**

|Function|CPU|DSP |MEM|
|:-------|:--|:---|:--|
|CPU.MISO|D6 |P1.3|2  |
|CPU.MOSI|D7 |P1.4|5  |
|CPU.CLK |D5 |P1.5|6  |

### Control
**Table: Control connections**

|Function|CPU|IO  |LVL|DSP |MEM|AMP   |
|:-------|:--|:---|:--|:---|:--|:-----|
|CPU.XCS |D4 |    |   |P1.7|   |      |
|CPU.XDCS|D3 |    |   |P1.6|   |      |
|CPU.CS  |D8 |    |   |    |1  |      |
|IO.MUTE |   |GPB0|   |    |   |MUTE.1|
|IO.RST  |   |GPB1|HV3|    |   |      |
|LVL.RST |   |    |LV3|P1.1|   |      |
|DSP.DREQ|D0 |    |   |    |   |      |

### Audio
**Table: Audio connections**

|Function  |DSP        |AMP   |AUX   |
|:---------|:----------|:-----|:-----|
|DSP.LEFT  |PHONE.LEFT |LEFT  |      |
|DSP.RIGHT |PHONE.RIGHT|RIGHT |      |
|DSP.GND   |PHONE.GND  |GND   |      |
|AMP.LEFT+ |           |LEFT+ |LEFT+ |
|AMP.LEFT- |           |LEFT- |LEFT- |
|AMP.RIGHT+|           |RIGHT+|RIGHT+|
|AMP.RIGHT-|           |RIGHT-|RIGHT-|

### User
**Table: User connections**

|Function      |AUX|IO  |
|:-------------|:--|:---|
|GND           |7  |GND |
|KEY_UP        |8  |GPA0|
|KEY_DOWN      |9  |GPA1|
|KEY_LEFT      |10 |GPA2|
|KEY_RIGHT     |11 |GPA3|
|KEY_HOME      |12 |GPA4|
|GND           |13 |GND |
|ENCODER_A     |14 |GPA5|
|ENCODER_B     |15 |GPA6|
|ENCODER_SWITCH|16 |GPA7|

		