# ParaTNC and ParaMETEO firmware version EA20, April 2023

// Please look into 'doc' directory for more documentation and user manuals
// WARNING: This readme could be sligtly outdated due to my lack of time :)

#### INTRODUCTION 

ParaTNC and ParaMETEO is a two APRS controller which has sligtly different target and different features. Source code in this repository called ParaTNC is a firmware to both of them. The name comes from the fact that ParaTNC was the first project. 
 
__ParaTNC__   supports key elements of what good APRS device should have:
   - Two directional KISS TNC (no init strings required).
   - WIDE1-1 digipeater, with an option to limit digipeating only to SSIDs 7, 8 and 9.
   - Weather station with support for various meteo sensors. Full list od supported devices in point section below.
   - Extensive telemetry with an information about the count of receved, transmitted and digipeated frames plus status of weather sensors.
   - Support for VE.Direct serial protocol used in Victron PV charging controllers. The data about currents and voltages in the PV system are transmitted using APRS telemetry.
   - Support for UMB Binary porotocol as a mater.
   - Support for Modbus RTU
   - Three different grounds separated one from the another (Controller, weather sensors, analog interface to FM radio)

__ParaMETEO__   is full outdoor and easy to install, complete APRS device which can be supplied with power either from mains and PV system.. It supports the same list of feature than ParaTNC and:
   - integrated MPPT charger for 12V / 9Ah AGM battery enclosed in the same case. 
   - integrated GSM module and support for APRS-IS communication (can run RX-igate)
   - support for MAX31865 amplifier for 2, 3 or 4-wire PT100/PT1000 temperature sensor
   - 7.5V dc converter to supply HT radio installed in the case for APRS radio network

***Please remember than ParaTNC firmware no longer runs on STM32VLDSICOVERY board!***

ParaTNC PCB could be manufactured used a set of generber files locates in ./hardware/ directory a long with schematics and layout renders (top and bottom layer). 

#### LICENSING

ParaTNC software and hardware are licensed under terms included to the source code in 'LICENSE' file

#### SUPPORTED METEO SENSORS
   + Wind sensors: 
     - Davis 6410 or any another analogue anemometer with resisntance as a direction output and impulses as a windspeed
     - Lufft V200A/Ventus or any other UMB compatible device.
     - Any Modbus-RTU sensor
  + Pressure Sensors:
     - MS5611
     - Bosh BMP280
     - Any UMB protocol compatible device
     - Any Modbus-RTU sensor
  + Temperature sensors:
     - Dallas One wire DS18B20
     - Two, three or four wire PT100/PT1000
     - Any UMB protocol compatible device
     - Any Modbus-RTU sensor
  + Humidity Sensors:
     - DHT22
     - Bosh BMP280
     - Any Modbus-RTU sensor (F&F MB-AHT1 is recommended)


#### SUPPORTED FEATURES OF VE.Direct PROTOCOL
Most of Victron devices have a support both for binary and text serial protocol. By default the text procol (VE.Direct) is 
always enabled and a device will send from its own telegrams each couple of seconds. The communication via VE.Direct is 
avaliable through dedicated socket on the charging controller which is just 3.3V TTL levels UART, so no external ICs is 
required to connect the PV controller to an evaluation board. In the MPPT series the comm socket is located on the bottom 
side of the charging controller below the fuse holder.

Exact pinout of the VE.Direct comm socket is as follows, assuming that terminal screws are facing down:
 + Ground
 + TX, data from host to the PV controller
 + RX, data from PV controller to the host

The controller sends a lot of different data which cannot be completely transmitted through APRS network due to radioprotocol 
limitations. Only these parameters are transmitted:
 + Battery Current (charging as positive, discharging as negative) in third channel of telemetry.
 + Battery Voltage as fourth telemetry channel.
 + PV cell Voltage as fifth telemetry channel.
 + Charging Controller status (like current charging mode) and minimum/maximum battery current in last 10 minutes.   
 + Error Codes if any (short circuit, overheat, overvoltage on PV or battery input)

*** Please note that VE.Direct is supported only in ParaTNC. In theory it is possible to connect any Victron Energy controller to ParaMETEO, but it has no sense in practice. ParaMETEO has it's own integrated chaging controller. ***

#### TELEMETRY
If the VE.Direct protocol suppot is disabled the ParaTNC uses telemetry packets to send internal statistics and
general information about communication with sensors. Telemetry works in 10 minutes cycle, which means that packets are sent
every 10 minutes and they represents a state for the time between one and another. 

Analog channels (ParaTNC):

  - 1st Channel: Number of packets received in 10 minutes.
  - 2nd Channel: Number of packets transmitted in 10 minutes (digi-ed + generated internally by the controller)
  - 3rd Channel: Number of digipeated packets in 10 minutes.
  - 4th Channel: Number of packet received from Host by KISS protocol
  - 5th Channel: Optional temperature from Dalls One Wire sensor.


Digital Channels (ParaTNC):
  1. DS_QF_FULL	- Quality Factor for One Wire temperature sensor is FULL
  2. DS_QF_DEGRAD	- Quality Factor for One Wire temperature sensor is DEGRADATED
  3. DS_QF_NAVBLE	- Quality Factor for One Wire temperature sensor is NOT AVALIABLE
  4. MS_QF_NAVBLE	- Quality Factor for MS5611 pressure sensor ins NOT AVALIABLE
  5. DHT_QF_NAVBLE	- Quality Factor for DHT22 humidity & temperature is NOT AVALIABLE
  6. WIND_QF_DEGR	- LaCrosse TX20 anemometer driver dropped at least one measuremenet due to excesive slew rate.
  7. WIND_QF_NAVB	- 

Explantion of Quality Factors: 

Each measuremenets signal is kept along with the Quality Factor which represents the sensor condition
and value validity. If the QF is set to FULL it means that no communication problems happened between one telemetry cycle and another
DEGRADATED is set if at least once the communication problem happened (CRC calculation fail, timeout etc). NOT AVALIABLE is set
if all communication atempts failed and no valid measuremenet value is avaliable. By default each Quality Factor is set to 
NOT AVALIABLE after the telemetry packets are sent.  

#### CONFIGURATION
At this point ParaTNC is delivered in form of source code which needs to be manually compiled by the user. Most options are configured by #define in './include/station_config.h' and then hard-coded by the C preprocessor during compilation. An example file consist a lot of comments which explains what is what, but generally an user can choose there which mode should be enabled:
	
		Hardware Revision B & C:
	-> KISS TNC
	-> KISS TNC + DIGI
	-> KISS TNC + DIGI + METEO
	-> KISS TNC + VICTRON + DIGI + METEO
	-> KISS TNC + VICTRON + DIGI
	-> KISS TNC + UMB-MASTER + DIGI + METEO
	

Harcoded values from station_config.h are then used as defaults. Normally firmware loads config from separate sections in flash memory. Configuration is stored twice in two section, to enable easy and safe configuration change from host pc. Each section has a programming counter and CRC value. Software calculates checksum and use last good configuration. 

			-> Defaults (only ParaTNC): 0x0801E000
			-> First section: 0x0801E800
			-> Second section: 0x0801F000

The KISS modem runs on default speed of 9600 bps. Telemetry is enabled by default and it will
trasmit channels values each 10 minutes and full channel descriptions each 70 minutes.

 
#### TOOLCHAIN
To build the ParaTNC software 'GNU ARM Embedded Toolchain' is required. This set contains gcc-arm-none-eabi compiler, gdb debugger, linker, HEX generator and set of libraries. ParaTNC is developed in Xubuntu 16.04LTS,  20.04LTS and 22.04 using toolchain in version 2018-q2. Please take note that You have to use 64-bit version of the operation system as the 32-bit variant of the toolchain is not avaliable. 

***This firmware is developed using Eclipse IDE for Embedded C/C++ Developers in Version: 2023-03 (4.27.0)***

Exact GCC version is 7.3.1 and it present itself with a string like this below
gcc version 7.3.1 20180622 (release) [ARM/embedded-7-branch revision 261907] (GNU Tools for Arm Embedded Processors 7-2018-q2-update) 

Self conatined TAR.BZ2 archive with all required tools can be found here:
https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads
in 'What's new in 7-2018-q2-update' section pulled down from the list located in the middle of this site. 
Alternatively You can use this link: http://pogoda.cc/d/gcc-arm-none-eabi-7-2018-q2-update-linux.tar.bz2

After download the content of this archive has to be uncompressed into: /usr/local/bin/gcc-arm-none-eabi-7-2018-q2-update/
so the structure should looks like this

	mateusz@mateusz-ThinkCentre-M720q:/usr/local/bin/gcc-arm-none-eabi-7-2018-q2-update$ ls -la
	total 24
	drwxrwxr-x 6 mateusz mateusz 4096 paź 10 08:35 .
	drwxr-xr-x 7 root    root    4096 paź 10 18:06 ..
	drwxr-xr-x 6 mateusz mateusz 4096 cze 22  2018 arm-none-eabi
	drwxr-xr-x 2 mateusz mateusz 4096 cze 22  2018 bin
	drwxr-xr-x 3 mateusz mateusz 4096 cze 22  2018 lib
	drwxr-xr-x 4 mateusz mateusz 4096 cze 22  2018 share
	mateusz@mateusz-ThinkCentre-M720q:/usr/local/bin/gcc-arm-none-eabi-7-2018-q2-update$ 

Both a makefile and an Eclipse project are configured to look for toolchain in this directory. In some cases to perform a debugging in Elipse You will have to install libncurses5 library which is required to start GDB. To check if this is a case try to start the debugger manually by issuing such command in the prompt

 '/usr/local/bin/gcc-arm-none-eabi-7-2018-q2-update/bin/arm-none-eabi-gdb --version'

If such result will be printed in the console, libncurses5 must be installed, prefferably using system package manager like aptitude in Debian/Ubuntu

 'libraries: libncurses.so.5: cannot open shared object file: No such file or directory'

To start debugging session in Eclipse you must create new 'GDB OpenOCD Debugging' configuration and set paths to OpenOCD and GDB in 'Debugger' tab. OpenOCD usually sits in '/usr/bin/openocd', the path to GDB is shown in the paragraph before. Remember to set 'Config Options:' to tell the OpenOCD what JTAG adapter/debugger is used in Your setup. If You're using ST-Link/V2 paste '-f interface/stlink-v2.cfg -f target/stm32f1x_stlink.cfg' 

#### DOWNLOADING THE SOURCE CODE AND COMPILING IT

When everything is installed the reporistory can be cloned to local harddrive by using a command
 'git clone https://github.com/sp8ebc/ParaTNC'

The example config file is named 'station_config_example.h' and should be edited and then renamed to 
'station_config.h'. When everything is configured it is a time to go to 'Debug' directory and invoke
command 'make' there. The source should be automatically compiled and new hex file 'ParaTNC.hex'
should appear in the same directory. Please do not use Release configuration!! It is screwed and produces an
application which doesn't works exactly as it should because of optimalization configuration.

	Finished building target: ParaTNC.elf
 
	Invoking: Cross ARM GNU Create Flash Image
	arm-none-eabi-objcopy -O ihex "ParaTNC.elf"  "ParaTNC.hex"
	Finished building: ParaTNC.hex
 
	Invoking: Cross ARM GNU Print Size
	arm-none-eabi-size --format=berkeley "ParaTNC.elf"
	   text	   data	    bss	    dec	    hex	filename
	  50542	    576	   5544	  56662	   dd56	ParaTNC.elf
	Finished building: ParaTNC.siz
 

	22:29:38 Build Finished (took 13s.231ms)

#### LOADING THE HEX FILE USING THE SERIAL BOOTLOADER

If You don't have a JTAG programmer/debugger or You just not want to or can't use it for any reason, You can choose
Internal Serial Bootloader provided by STMicroelectronics. It's code is stored in mask ROM within microcontroler
and can be used anytime, and in scope of ParaTNC it practically cannot be disabled or locked. Please remember that
this option is avaliable only if You're using ParaTNC PCB as STM32VLDISCOVERY board doesn't have TTL-RS232 converter.

To use serial bootloader You need:
  - RJ45 to DB9 Cisco style RS232 cable
  - FlashLoader Demonstrator software provided by STMicroelectronics for free
  
The required software can be downloaded from this link: https://www.st.com/en/development-tools/flasher-stm32.html
It is advised to read the user manual for this tool before proceding further. To jump to the bootloader You shall 
disconect power to ParaTNC, then use something to short the jumper located at the left of JTAG connector (top-left
corner of PCB) and with jumper shorted reconnect the power supply. If procedure success You shall NOT hear 
relay clicking and LEDs blinking for a short while. After the power supply is connected and micro stays in bootloader
you can disconnect the jumper and start the FlashLoader software to download the HEX file to micro. After process is done
you should do a cold reset without jumper shorter. 

#### LOADING THE HEX FILE INTO STM32VLDISCOVERY BOARD
The STM32VLDISCOVERY board has an ST-Link/V1 programmer-debugger on board which can be used to load a HEX file.
This ST-Link appears normally as a mass storage device which makes in unusable to be used by HEX loadin software
in Linux (as the device will be 'blocked' by the mass-storage driver). To workaround this problem, a configuration
of modprobe daemon should be changed to ignore the ST-Link and not load any driver for it.

This is done by invoking a command:
 'sudo echo "options usb-storage quirks=483:3744:i" >> /etc/modprobe.d/stlink_v1.conf'
What will create a new file called 'stlink_v1.conf' in modprobe directory. After the system reboot changes should
be applied and the programmer should be free to go. The kernel log should looks somehow like this below

	[90639.895886] usb 2-1.1: new full-speed USB device number 13 using ehci-pci
	[90639.990288] usb 2-1.1: New USB device found, idVendor=0483, idProduct=3744
	[90639.990294] usb 2-1.1: New USB device strings: Mfr=1, Product=2, SerialNumber=3
	[90639.990296] usb 2-1.1: Product: STM32 STLink
	[90639.990298] usb 2-1.1: Manufacturer: STMicroelectronics
	[90639.990300] usb 2-1.1: SerialNumber: Qÿr\xffffff86RV%X\xffffffc2\xffffff87
	[90639.990796] usb-storage 2-1.1:1.0: USB Mass Storage device detected
	[90639.992973] usb-storage 2-1.1:1.0: device ignored

The next step is to install texane-stlink which supports the ST-Link/V1 programmer and can be used to read an write
the flash memory. Installation is quite straight forward

	'git clone git://github.com/texane/stlink.git'
	'cd stlink.git'
	'make'
	'cd build/Relase'
	'sudo cp st-* /usr/bin'

At the end the HEX file can be loaded into the microcontroler by typing a command

	'sudo st-flash --format ihex write /dev/sr0 ParaTNC.hex' 

 #### LOADING THE HEX FILE INTO ParaTNC or ParaMETEO TARGET USING STLINK/V2

	mateusz@mateusz-ThinkCentre-M720q:~/Documents/___STM32/ParaTNC/STM32L476_ParaMETEO$ st-flash erase
	st-flash 1.7.0-120-gbeffed4
	/usr/local/stlink/chips: No such file or directory
	2023-11-17T07:43:18 INFO common.c: L47x/L48x: 96 KiB SRAM, 1024 KiB flash in at least 2 KiB pages.
	Mass erasing

 and then programming itself

	mateusz@mateusz-ThinkCentre-M720q:~/Documents/___STM32/ParaTNC/STM32L476_ParaMETEO$ st-flash --format ihex write ParaTNC.hex
	st-flash 1.7.0-120-gbeffed4
	/usr/local/stlink/chips: No such file or directory
	2023-11-17T07:44:47 INFO common.c: L47x/L48x: 96 KiB SRAM, 1024 KiB flash in at least 2 KiB pages.
	2023-11-17T07:44:47 INFO common.c: Attempting to write 111504 (0x1b390) bytes to stm32 address: 134217728 (0x8000000)

	2023-11-17T07:44:48 INFO common.c: Finished erasing 55 pages of 2048 (0x800) bytes
	2023-11-17T07:44:48 INFO common.c: Starting Flash write for F2/F4/F7/L4
	2023-11-17T07:44:48 INFO flash_loader.c: Successfully loaded flash loader in sram
	2023-11-17T07:44:48 INFO flash_loader.c: Clear DFSR
	2023-11-17T07:44:48 INFO flash_loader.c: Clear CFSR
	2023-11-17T07:44:48 INFO flash_loader.c: Clear HFSR
	2023-11-17T07:44:51 INFO common.c: Starting verification of write complete
	2023-11-17T07:44:52 INFO common.c: Flash written and verified! jolly good!

 #### LOADING THE HEX FILE INTO ParaTNC or ParaMETEO TARGET USING ROM BOOTLOADER AND STM32FLASH software

 Examples:
        Get device information:
                stm32flash /dev/ttyS0
          or:
                stm32flash /dev/i2c-0

        Write with verify and then start execution:
                stm32flash -w filename -v -g 0x0 /dev/ttyS0

        Read flash to file:
                stm32flash -r filename /dev/ttyS0

        Read 100 bytes of flash from 0x1000 to stdout:
                stm32flash -r - -S 0x1000:100 /dev/ttyS0

        Start execution:
                stm32flash -g 0x0 /dev/ttyS0

