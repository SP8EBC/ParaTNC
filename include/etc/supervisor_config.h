/*
 * supervisor_config.h
 *
 *  Created on: Mar 10, 2025
 *      Author: mateusz
 */

#ifndef ETC_SUPERVISOR_CONFIG_H_
#define ETC_SUPERVISOR_CONFIG_H_


#define SUPERVISOR_CONFIG(ENTRY)										\
			/* Thread or library,		timeout seconds - max 65535 */	\
	ENTRY(	SEND_WX,					180)							\
	ENTRY(	MAIN_LOOP,					10)								\
	ENTRY(	TASK_ONE_MIN,				77)								\
	ENTRY(	TASK_ONE_SEC,				4)								\
	ENTRY(	TASK_TWO_SEC,				5)								\
	ENTRY(	TASK_TEN_SEC,				15)								\
	ENTRY(	TASK_POWERSAV,				15)								\
	ENTRY(	EVENT_NEW_RF,				65530)							\
	ENTRY(	EVENT_APRSIS_MSG_TRIG,		300)							\
	ENTRY(	EVENT_SRL_GSM_RX_DONE,		300)							\
	ENTRY(	EVENT_SRL_GSM_TX_DONE,		300)							\
	ENTRY(	EVENT_SRL_KISS_RX_DONE,		65530)							\
	ENTRY(	EVENT_SRL_KISS_TX_DONE,		65530)							\




#endif /* ETC_SUPERVISOR_CONFIG_H_ */
