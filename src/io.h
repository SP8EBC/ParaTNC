/*
 * io.h
 *
 *  Created on: 11.06.2020
 *      Author: mateusz
 */

#ifndef IO_H_
#define IO_H_

void io_oc_init(void);
void io_oc_output_low(void);
void io_oc_output_hiz(void);

void io_ext_watchdog_config(void);
void io_ext_watchdog_service(void);


#endif /* IO_H_ */
