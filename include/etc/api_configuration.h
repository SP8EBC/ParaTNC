/*
 * api_configuration.h
 *
 *  Created on: Apr 30, 2022
 *      Author: mateusz
 */

#ifndef API_CONFIGURATION_H_
#define API_CONFIGURATION_H_

// API cycle are fired once every minute and counts from zero
// up to 100 and then starts again. These defines controls how
// often in minutes API communication is triggered

#define API_STATUS_CALL_CYCLE_INTERVAL			20
#define API_MEASUREMENTS_CALL_CYCLE_INTERVAL	5

#endif /* API_CONFIGURATION_H_ */
