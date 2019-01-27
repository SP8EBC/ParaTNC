/*
 * _dht22.h
 *
 *  Created on: 17.04.2018
 *      Author: mateusz
 */

#ifndef INCLUDE_DRIVERS__DHT22_H_
#define INCLUDE_DRIVERS__DHT22_H_

#include "stdint.h"

#define DHT22_START_SIG_DURATION 					200
#define DHT22_WAITING_FOR_START_RESP_DURATION		12
#define DHT22_LOW_LEVEL_BEFORE_BIT					10
#define DHT22_MAX_ZERO_DURATION						20
#define DHT22_INTERRUPT_DURATION					40

#define DHT22_PIN_PORT            GPIOC
#define DHT22_PIN_CLOCK           RCC_APB2Periph_GPIOC
#define DHT22_PIN_PIN             GPIO_Pin_4

#define DHT22_STATE_IDLE		10
#define DHT22_STATE_COMMS		11
#define DHT22_STATE_DATA_RDY	12	// data has been rxed from a sensor and it is ready for decoding
#define DHT22_STATE_DATA_DECD	13	// data has been decoded
#define DHT22_STATE_TIMEOUT		14
#define DHT22_STATE_DONE		15
#define DHT22_STATE_COMMS_IRQ	16

typedef enum dht22QF {
	DHT22_QF_UNKNOWN = 0,
	DHT22_QF_FULL,
	DHT22_QF_DEGRADATED,
	DHT22_QF_UNAVALIABLE
} dht22QF;

typedef struct dht22Values {
	uint8_t humidity;
	uint16_t scaledTemperature;
	dht22QF qf;
}dht22Values;

extern uint8_t dht22State;

#ifdef __cplusplus
extern "C" {
#endif

void dht22_init(void);
void dht22_comm(dht22Values *data);
void dht22_decode(dht22Values *data);
void dht22_timeout_keeper(void);
void dht22_irq_handler(void);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_DRIVERS__DHT22_H_ */
