/*
 * pwr_save.h
 *
 *  Created on: Aug 22, 2021
 *      Author: mateusz
 */

#ifndef PWR_SAVE_H_
#define PWR_SAVE_H_

#include "station_config_target_hw.h"

/**
 * This header file defines all functions related to powersaving, switching between different power states,
 * stopping cpu core etc. Generally they are few power states as below
 *
 * /------------------------------------------------------------\
 * | State |  CPU  |  +5V_S  |  +5V_R and VBATT_SW_R  |  +4V_G  |
 * |       |       |         |                        |         |
 * |       |       | sensors |   Internal / external  |   GPRS  |
 * |       |       |         |        VHF Radio       |         |
 * =------------------------------------------------------------=
 * |   C0  |Running|   ON    |           ON           |   ON    |
 * |   C1  |Running|   ON    |           ON           |   OFF   |
 * |   C2  |Running|   OFF   |           ON           |   OFF   |
 * |   C3  |Running|   OFF   |           ON           |   ON    |
 * |   M4  |Running|   ON    |           OFF          |   OFF   |
 * |   I5  |Running|   OFF   |           OFF          |   OFF   |
 * |   L6  | Stop2 |   OFF   |           OFF          |   ON    |
 * |   L7  | Stop2 |   OFF   |           OFF          |   OFF   |
 * \------------------------------------------------------------/
 *
 * 		C = modes with communication enabled
 * 		M = mode with measuremenet only w/o any communication
 * 		I = idle / initialization mode with anything disabled
 * 		L = low power consumption modes with CPU halted in STOP2 mode
 *
 * 	Stop2 mode is a power saving mode defined in STM paperwork (DM0083560
 * 	aka Reference Manual RM0351). This mode halts CPU core completely and
 * 	disable HCLK, AHB1 & AHB2 buses, but preserves StaticRAM and registers
 * 	content. 32k-LSE and RTC works normally and wakeups the core every
 * 	programmed amount of seconds.
 *
 * 	C0 - Mode with everything avaliable. Used when station shall works
 * 	as APRS digi / igate and is (probably) powered from mains instead
 * 	of PV
 *
 * 	C1 - Mode with everything except GPRS modem is powered. This is
 * 	default mode when station is powered from PV and works as DIGI.
 * 	Digi required constant participation in radio channel traffic
 * 	so VHF radio must operates constantly.
 *
 * 	C2 - Simmilar to C1 but with all meteo sensors disabled. This helps
 * 	save some mA of current drain (but presumably radio will consume more
 * 	than sensors)
 *
 * 	M4 - Measurement only mode when only meteo sensors are enabled
 *
 * 	I5 - Idle / initialization mode when only micro is operating normally but
 * 	all aux voltages are turned off. This is mode CPU starts after poweron/reset
 * 	and wakes up from Stop2
 *
 * 	L6 - This is Low power STOP2 mode with GSM radio kept in standby mode. +4V_G
 * 	is powered on but GSM radio is left in most power efficient mode available.
 * 	This is used when WX packet interval is shorter than 5 minutes. In such case
 * 	GSM modem reinitialization and tuning & logging into network will consume more
 * 	power than keeping modem on standby.
 *
 *	L7 - Deep sleep mode with everything powered off and CPU kept in STOP2 mode.
 *
 *
 *
 *	Transitions between states depends on configuration and value of config_data_powersave_mode_t
 *
 *  ====================================================================================================================================|
 *	|  	Mode				|	Powersave Mode	|																			 			|
 *	|=======================|===================|=======================================================================================|
 *	|	DIGI  				|	don't care		|	Always stays in C2 no matter of how config_data_powersave_mode_t is set	 			|
 *	|	DIGI + WX			|	PWSAVE_NONE		|		C1																	 			|
 *	|	DIGI + WX			|	PWSAVE_NORMAL	|		C2 --- (1 minute before WX frame)---> C1 -> C2						 			|
 *	|	DIGI + WX			|	PWSAVE_AGGRESV	|		C2 --- (1 minute before WX frame)---> C1 -> C2									|
 *	|	DIGI + WX + GSM		|	PWSAVE_NONE		|		C0																	 			|
 *	|	DIGI + WX + GSM		|	PWSAVE_NORMAL	|		C2 --- (1 minute before WX frame)---> C0 -> C2  ; if WX_INTERVAL >= 5 minutes	|
 *	|	DIGI + WX + GSM		|	PWSAVE_NORMAL	|		C3 --- (1 minute before WX frame)---> C0 -> C3  ; if WX_INTERVAL < 5 minutes	|
 *	|	DIGI + WX + GSM		|	PWSAVE_AGGRESV	|		C2 --- (1 minute before WX frame)---> C0 -> C2  ; no matter WX_INTERVAL 		|
 *	|	WX 					|	PWSAVE_NONE		|		C2 --- (30 seconds before WX frame)---> C1 -> C2						 		|
  *	|	WX 					|	PWSAVE_NORMAL	|		M4 --- (30 seconds before WX frame)---> C1 -> M4						 		|
  *	|	WX 					|	PWSAVE_AGGRESV	|		L7 --- (1 minute before WX frame)---> M4 --- (30 sec before)---> C1 -> L7 		|
  *	====================================================================================================================================|
 *
 */

#if defined(STM32L471xx)

void pwr_save_init(void);
void pwr_save_enter_stop2(void);
void pwr_save_switch_c0(void);
void pwr_save_switch_c1(void);
void pwr_save_switch_c2(void);
void pwr_save_switch_c3(void);
void pwr_save_switch_m4(void);
void pwr_save_switch_i5(void);
void pwr_save_switch_l6(void);
void pwr_save_switch_l7(void);



#endif


#endif /* PWR_SAVE_H_ */
