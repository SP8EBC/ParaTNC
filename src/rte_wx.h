/*
 * rte_wx.h
 *
 *  Created on: 26.01.2019
 *      Author: mateusz
 */

#include "drivers/_dht22.h"
#include "drivers/dallas.h"
#include "drivers/ms5611.h"


#ifndef RTE_WX_H_
#define RTE_WX_H_

extern float rte_wx_temperature_dallas, rte_wx_temperature_dallas_valid;
extern float rte_wx_temperature, rte_wx_temperature_valid;
extern float rte_wx_pressure, rte_wx_pressure_valid;

extern dht22Values rte_wx_dht, rte_wx_dht_valid;

extern DallasQF rte_wx_dallas_qf;
extern ms5611_qf_t rte_wx_ms5611_qf;


#endif /* RTE_WX_H_ */
