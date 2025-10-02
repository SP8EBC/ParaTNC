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
	ENTRY(	TASK_ONE_SEC,				2)								\
	ENTRY(	TASK_TWO_SEC,				3)								\
	ENTRY(	TASK_TEN_SEC,				11)								\
	ENTRY(	TASK_POWERSAV,				11)								\
	ENTRY(	EVENT_NEW_RF,				300)							\




#endif /* ETC_SUPERVISOR_CONFIG_H_ */
