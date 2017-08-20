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

void TimerConfig(void);
void TIM2Delay(char delay);
void TIM2DelayDeConfig(void);

#ifdef __cplusplus
}
#endif

#endif /* TIMERCONFIG_H_ */
