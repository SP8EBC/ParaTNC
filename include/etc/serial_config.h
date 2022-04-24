/*
 * serial_config.h
 *
 *  Created on: Apr 24, 2022
 *      Author: mateusz
 */

#ifndef SERIAL_CONFIG_H_
#define SERIAL_CONFIG_H_

#define RX_BUFFER_1_LN 384
#define TX_BUFFER_1_LN 384

#define RX_BUFFER_2_LN 96
#define TX_BUFFER_2_LN 96

#define RX_BUFFER_3_LN 768
#define TX_BUFFER_3_LN 768

#define SEPARATE_RX_BUFF
#define SEPARATE_TX_BUFF

#define SRL_TX_DELAY_IN_MS	30
#define SRL_DEFAULT_RX_TIMEOUT_IN_MS 1200

#endif /* SERIAL_CONFIG_H_ */
