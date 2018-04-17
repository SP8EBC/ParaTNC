/*
 * _dht22.h
 *
 *  Created on: 17.04.2018
 *      Author: mateusz
 */

#ifndef INCLUDE_DRIVERS__DHT22_H_
#define INCLUDE_DRIVERS__DHT22_H_

#define DHT22_START_SIG_DURATION 					200
#define DHT22_WAITING_FOR_START_RESP_DURATION		12
#define DHT22_LOW_LEVEL_BEFORE_BIT					10
#define DHT22_MAX_ZERO_DURATION						6
#define DHT22_MIN_ONE_DURATION						10
#define DHT22_INTERRUPT_DURATION					40

#define DHT22_PIN_PORT            GPIOC
#define DHT22_PIN_CLOCK           RCC_APB2Periph_GPIOC
#define DHT22_PIN_PIN             GPIO_Pin_4

typedef enum dht22QF {
	DHT22_QF_FULL,
	DHT22_QF_DEGRADATED,
	DHT22_QF_UNAVALIABLE
} dht22QF;

typedef struct dht22Values {
	uint8_t humidity;
	uint16_t scaledTemperature;
	dht22QF qf;
}dht22Values;

#ifdef __cplusplus
extern "C" {
#endif
void dht22_init(void);
void dht22_comm(dht22Values *data);
#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_DRIVERS__DHT22_H_ */
