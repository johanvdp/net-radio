# Internet radio
The basic design of a project that is just started...
The internet radio is used to play audio from radio station streams (http). Details such as the network location are configured (externally) and accessible to the user as favorites.

# Framework
The application is build using the
[Espressif IoT Development Framework](https://esp-idf.readthedocs.io/en/latest/index.html).

**Table of contents**

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

|   |1234567890123456|
|:-:|----------------|
|1  |Favorite        |
|2  |Title           |
|3  |Artist          |
|4  |Album           |

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
Maintain a representation of user actions.
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
Manage the transition between multiple pages. Making one page active and visible at one time. Each page provides its own strategy to:

+ Evaluate user input
+ Perform actions
+ Navigate to a different page
+ Update display

## Reader
Read data from network a stream and write to the buffer

## Buffer
+ Provide access to read and write methods.

## Player
+ Read from buffer (stream source)
+ Send to DSP (stream sink)

## Control
+ Provides debug interface
+ Interpret control commands

## I2C
+ Arbitrate usage
	+ IO
	+ LCD

## MEM
+ Provide memory access via VSPI

## DSP
+ Provide DSP access via HSPI

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
NodeMCU ESP-32S module.

+ 4M bytes(32M bit) flash memory
+ Built-in PCB antenna
+ CP2102 USB to serial controller
+ AMS1117 3.3V LDO

The NodeMCU module contains an ESP-32S module, which contains an ESP32 chip. Because the ESP32 chip is flexible in mapping functions and pins the following table clarifies their relationship.

**Table: ESP32 connections**

|ESP32 pin|ESP32 functions                                                                      |ESP-32S pin|NodeMcu pin|NodeMcu label|
|:--------|:------------------------------------------------------------------------------------|:----------|:----------|:------------|
|1        |VDDA                                                                                 |           |           |             |
|2        |LNA_IN                                                                               |           |           |             |
|3        |VDD3P3                                                                               |           |           |             |
|4        |VDD3P3                                                                               |           |           |             |
|5        |SENSOR_VP, GPIO36, ADC_PRE_AMP, ADC1_CH0, RTC_GPIO0                                  |4          |3          |SVP          |
|6        |SENSOR_CAPP                                                                          |           |           |             |
|7        |SENSOR_CAPN                                                                          |           |           |             |
|8        |SENSOR_VN, GPIO39, ADC1_CH3, ADC_PRE_AMP, RTC_GPIO3                                  |5          |4          |SVN          |
|9        |CHIP_PU, Chip Enable (Active High)                                                   |3          |2          |EN           |
|10       |VDET_1, GPIO34, ADC1_CH6, RTC_GPIO4                                                  |6          |5          |P34          |
|11       |VDET_2, GPIO35, ADC1_CH7, RTC_GPIO5                                                  |7          |6          |P35          |
|12       |32K_XP, GPIO32, ADC1_CH4, TOUCH9, RTC_GPIO9                                          |8          |7          |P32          |
|13       |32K_XN, GPIO33, ADC1_CH5, TOUCH8, RTC_GPIO8                                          |9          |8          |P33          |
|14       |GPIO25, DAC_1, ADC2_CH8, RTC_GPIO6, EMAC_RXD0                                        |10         |9          |P25          |
|15       |GPIO26, DAC_2, ADC2_CH9, RTC_GPIO7, EMAC_RXD1                                        |11         |10         |P26          |
|16       |GPIO27, ADC2_CH7, TOUCH7, RTC_GPIO17, EMAC_RX_DV                                     |12         |11         |P27          |
|17       |MTMS, GPIO14, ADC2_CH6, TOUCH6, RTC_GPIO16, HSPI-CLK, HS2_CLK, SD_CLK, EMAC_TXD2     |13         |12         |P14          |
|18       |MTDI, GPIO12, ADC2_CH5, TOUCH5, RTC_GPIO15, HSPIQ, HS2_DATA2, SD_DATA2, EMAC_TXD3    |14         |13         |P12          |
|19       |VDD3P3_RTC                                                                           |           |           |             |
|20       |MTCK, GPIO13, ADC2_CH4, TOUCH4, RTC_GPIO14, HSPID, HS2_DATA3, SD_DATA3, EMAC_RX_ER   |16         |15         |P13          |
|21       |MTDO, GPIO15, ADC2_CH3, TOUCH3, RTC_GPIO13, MTDO, HSPICS0, HS2_CMD, SD_CMD, EMAC_RXD3|23         |23         |P15          |
|22       |GPIO2, ADC2_CH2, TOUCH2, RTC_GPIO12, HSPIWP, HS2_DATA0, SD_DATA0                     |24         |24         |P2           |
|23       |GPIO0, ADC2_CH1, TOUCH1, RTC_GPIO11, CLK_OUT1, EMAC_TX_CLK                           |25         |25         |P0           |
|24       |GPIO4, ADC2_CH0, TOUCH0, RTC_GPIO10, HSPIHD, HS2_DATA1, SD_DATA1, EMAC_TX_ER         |26         |26         |P4           |
|25       |GPIO16, HS1_DATA4, U2RXD, EMAC_CLK_OUT                                               |27         |27         |P16          |
|26       |VDD_SDIO                                                                             |           |           |             |
|27       |GPIO17, HS1_DATA5, U2TXD, EMAC_CLK_OUT_180                                           |28         |28         |P17          |
|28       |SD_DATA_2, GPIO9, SPIHD, HS1_DATA2, U1RXD                                            |17         |16         |SD2          |
|29       |SD_DATA_3, GPIO10, SPIWP, HS1_DATA3, U1TXD                                           |18         |17         |SD3          |
|30       |SD_CMD, GPIO11, SPICS0, HS1_CMD, U1RTS                                               |19         |18         |CMD          |
|31       |SD_CLK, GPIO6, SPICLK, HS1_CLK, U1CTS                                                |20         |20         |CLK          |
|32       |SD_DATA_0, GPIO7, SPIQ, HS1_DATA0, U2RTS                                             |21         |21         |SD0          |
|33       |SD_DATA_1, GPIO8, SPID, HS1_DATA1, U2CTS                                             |22         |22         |SD1          |
|34       |GPIO5, VSPICS0, HS1_DATA6, EMAC_RX_CLK                                               |29         |29         |P5           |
|35       |GPIO18, VSPICLK, HS1_DATA7                                                           |30         |30         |P18          |
|36       |GPIO23, VSPID, HS1_STROBE                                                            |37         |37         |P23          |
|37       |VDD3P3_CPU                                                                           |           |           |             |
|38       |GPIO19, VSPIQ, U0CTS, EMAC_TXD0                                                      |31         |31         |P19          |
|39       |GPIO22, VSPIWP, U0RTS, EMAC_TXD1                                                     |36         |36         |P22          |
|40       |U0RXD, GPIO3, CLK_OUT2                                                               |34         |34         |RX           |
|41       |U0TXD, GPIO1, CLK_OUT3, EMAC_RXD2                                                    |35         |35         |TX           |
|42       |GPIO21, VSPIHD, EMAC_TX_EN                                                           |33         |33         |P21          |
|43       |VDDA                                                                                 |           |           |             |
|44       |XTAL_N                                                                               |           |           |             |
|45       |XTAL_P                                                                               |           |           |             |
|46       |VDDA                                                                                 |           |           |             |
|47       |CAP2                                                                                 |           |           |             |
|48       |CAP1                                                                                 |           |           |             |

The following table shows the function to terminal mapping for this application.

**Table: CPU terminals**

|Group  |Terminal|Function|Comment                 ||Group  |Terminal|Function|Comment                 |
|:------|:-------|:-------|------------------------||:------|:-------|:-------|------------------------|
|Power  |3V3     |        |                        ||Power  |GND     |        |                        |
|       |EN      |CHIP_PU |chip enable, active high||VSPI   |P23     |VSPID   |VSPI MOSI               |
|       |SVP     |        |                        ||       |P22     |        |                        |
|       |SVN     |        |                        ||Serial |TX      |        |in-use USB-to-serial    |
|       |P34     |        |                        ||Serial |RX      |        |in-use USB-to-serial    |
|       |P35     |        |                        ||       |P21     |        |                        |
|       |P32     |        |                        ||Power  |GND     |        |                        |
|       |P33     |        |                        ||VSPI   |P19     |VSPIQ   |VSPI MISO               |
|       |P25     |        |                        ||VSPI   |P18     |VSPICLK |VSPI SCK                |
|       |P26     |        |                        ||Control|P5      |VSPICS0 |MEM CS                  |
|Control|P27     |GPIO27  |DSP DREQ                ||I2C    |P17     |GPIO17  |I2C SCL                 |
|HSPI   |P14     |HSPI-CLK|HSPI SCK                ||I2C    |P16     |GPIO16  |I2C SDA                 |
|HSPI   |P12     |HSPIQ   |HSPI MISO               ||Control|P4      |GPIO4   |DSP XCS                 |
|Power  |GND     |        |                        ||       |P0      |        |strapping pin, boot mode|
|HSPI   |P13     |HSPID   |HSPI MOSI               ||       |P2      |GPIO2   |blue LED, active high   |
|       |SD2     |        |in-use 4MB flash        ||Control|P15     |HSPICS0 |DSP XDCS                |
|       |SD3     |        |in-use 4MB flash        ||       |SD1     |        |in-use 4MB flash        |
|       |CMD     |        |in-use 4MB flash        ||       |SD0     |        |in-use 4MB flash        |
|Power  |5V      |        |                        ||       |CLK     |        |in-use 4MB flash        |

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

+ 3V3 I/O

**Table: DSP terminals P1**

|Group  |Terminal     |Function                                                     |
|:------|:------------|:------------------------------------------------------------|
|Control|P1.1         |RST reset input (active low)                                 |
|Control|P1.2         |DREQ data request output (buffer can take 32 bytes when high)|
|HSPI   |P1.3         |SO serial output (active when XCS low)                       |
|HSPI   |P1.4         |SI serial input (sampled on rising edge SCK)                 |
|HSPI   |P1.5         |SCK serial clock input (can be continuous or gated)          |
|Control|P1.6         |XDCS data interface chip select (active low)                 |
|Control|P1.7         |XCS control interface chip select (active low)               |
|Power  |P1.8         |3V3 output (500mA max)                                       |
|Power  |P1.9         |5V input                                                     |
|Power  |P1.10        |GND                                                          |

**Table: DSP terminals P2**

|Group  |Terminal     |Function                                                     |
|:------|:------------|:------------------------------------------------------------|
|       |P2           |not connected                                                |

**Table: DSP terminals LINE_IN**

|Group  |Terminal     |Function                                                     |
|:------|:------------|:------------------------------------------------------------|
|       |LINE_IN      |not connected                                                |

**Table: DSP terminals PHONE**

|Group  |Terminal     |Function                                                     |
|:------|:------------|:------------------------------------------------------------|
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
|User   |GPA0    |KEY_UP, input            |
|User   |GPA1    |KEY_DOWN, input          |
|User   |GPA2    |KEY_LEFT, input          |
|User   |GPA3    |KEY_RIGHT, input         |
|User   |GPA4    |KEY_HOME, input          |
|User   |GPA5    |ENCODER_A, input         |
|User   |GPA6    |ENCODER_B, input         |
|User   |GPA7    |ENCODER_SWITCH, input    |
|Control|GPB0    |MUTE, active low, output |
|Control|GPB1    |RESET, active low, output|

### LCD display
128x64 pixel LCD display module controlled via an MCP23017 16-bit I/O expander.

+ 5V
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

+ 3V3

**Table: MEM terminals**

|Group  |Terminal|Function          |
|:------|:-------|:-----------------|
|Control|1       |CS, active low    |
|VSPI   |2       |SO/SIO1           |
|       |3       |SIO2, pull-up     |
|Power  |4       |VSS               |
|VSPI   |5       |SI/SIO0           |
|VSPI   |6       |SCK               |
|       |7       |HOLD/SIO3, pull-up|
|Power  |8       |VCC               |

### AMP amplifier
TPA3110 2x15W class D audio amplifier module.

**Table: AMP terminals**

+ 12V

|Group  |Terminal |Function                              |
|:------|:--------|:-------------------------------------|
|Power  |POWER.1  |12V                                   |
|Power  |POWER.2  |GND                                   |
|Control|MUTE.1   |MUTE, active low &lt;0.8V, high &gt;2V|
|       |MUTE.2   |GND, not connected                    |
|Audio  |LINE_IN.1|LEFT                                  |
|Audio  |LINE_IN.2|GND                                   |
|Audio  |LINE_IN.3|RIGHT                                 |
|Audio  |LEFT.1   |LEFT+                                 |
|Audio  |LEFT.2   |LEFT-                                 |
|Audio  |RIGHT.1  |RIGHT+                                |
|Audio  |RIGHT.2  |RIGHT-                                |

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
|GND     |GND|GND|GND|GND|GND|GND|P1.10|Power.2|4  |
|12V     |12V|IN |   |   |   |   |     |Power.1|   |
|5V      |   |OUT|5V |HV |VCC|VCC|P1.9 |       |   |
|3V3     |   |   |   |LV |   |   |3V3  |       |8  |

### I2C
**Table: I2C connections**

|Function|CPU|LVL|IO |LCD|
|:-------|:--|:--|:--|:--|
|CPU.SDA |   |LV1|   |   |
|LVL.SDA |   |HV1|SDA|SDA|
|CPU.SCL |   |LV2|   |   |
|LVL.SCL |   |HV2|SCL|SCL|

### SPI
**Table: HSPI connections**

|Function     |CPU|DSP |
|:------------|:--|:---|
|CPU.HSPI.MISO|P12|P1.3|
|CPU.HSPI.MOSI|P13|P1.4|
|CPU.HSPI.CLK |P14|P1.5|
|             |   |    |

**Table: VSPI connections**

|Function     |CPU|MEM|
|:------------|:--|:--|
|CPU.VSPI.SIOO|P23|5  |
|CPU.VSPI.SIO1|P19|2  |
|CPU.VSPI.SIO2|P22|3  |
|CPU.VSPI.SIO3|P21|7  |
|CPU.VSPI.CLK |P18|6  |

### Control
**Table: Control connections**

|Function|CPU|IO  |LVL|DSP |MEM|AMP   |
|:-------|:--|:---|:--|:---|:--|:-----|
|DSP.XCS |P4 |    |   |P1.7|   |      |
|DSP.XDCS|P15|    |   |P1.6|   |      |
|MEM.CS  |P5 |    |   |    |1  |      |
|IO.MUTE |   |GPB0|   |    |   |MUTE.1|
|IO.RST  |   |GPB1|HV3|    |   |      |
|LVL.RST |   |    |LV3|P1.1|   |      |
|DSP.DREQ|P27|    |   |    |   |      |

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

		