/*
 * TimerConfig.h
 *
 *  Created on: 04.04.2017
 *      Author: mateusz
 */

#ifndef TIMERCONFIG_H_
#define TIMERCONFIG_H_

/* C++ detection */
#ifdef __cplusplus
extern "C" {
#endif

void TimerTimebaseConfig(void);

void TimerConfig(void);
void TIM2Delay(void);
void TIM2DelayDeConfig(void);


void TimerAdcDisable(void);
void TimerAdcEnable(void);

#ifdef __cplusplus
}
#endif

#endif /* TIMERCONFIG_H_ */
