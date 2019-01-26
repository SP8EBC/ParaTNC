/*
 * rte_wx.c
 *
 *  Created on: 26.01.2019
 *      Author: mateusz
 */


#include "rte_wx.h"

float rte_wx_temperature_dallas = 0.0f, rte_wx_temperature_dallas_valid = 0.0f;
float rte_wx_temperature = 0.0f, rte_wx_temperature_valid = 0.0f;
float rte_wx_pressure = 0.0f, rte_wx_pressure_valid = 0.0f;

dht22Values rte_wx_dht, rte_wx_dht_valid;		// quality factor inside this structure
DallasQF rte_wx_dallas_qf;
ms5611_qf_t rte_wx_ms5611_qf;


