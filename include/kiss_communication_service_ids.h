/*
 * kiss_communication_service_ids.h
 *
 *  Created on: Apr 9, 2023
 *      Author: mateusz
 */

#ifndef KISS_COMMUNICATION_SERVICE_IDS_H_
#define KISS_COMMUNICATION_SERVICE_IDS_H_


#define KISS_DATA	 				(uint8_t) 0x00

#define KISS_RESTART			(uint8_t) 0x11
#define KISS_RESTART_RESP		(uint8_t) 0x51

#define KISS_GET_VERSION_AND_ID		(uint8_t) 0x21
#define KISS_VERSION_AND_ID			(uint8_t) 0x61

#define KISS_READ_DID			(uint8_t) 0x22
#define KISS_READ_DID_RESP		(uint8_t) 0x62

#define KISS_READ_MEM_ADDR			(uint8_t) 0x23
#define KISS_READ_MEM_ADDR_RESP		(uint8_t) 0x63

#define KISS_PROGRAM_STARTUP_CFG		(uint8_t) 0x34
#define KISS_PROGRAM_STARTUP_CFG_RESP	(uint8_t) 0x74

#define KISS_GET_RUNNING_CONFIG 	(uint8_t) 0x35
#define KISS_RUNNING_CONFIG			(uint8_t) 0x75

#define KISS_ERASE_STARTUP_CFG		(uint8_t) 0x37
#define KISS_ERASE_STARTUP_CFG_RESP	(uint8_t) 0x77

//#define KISS_CONFIG_CRC			(uint8_t) 0x24
//#define KISS_CONFIG_CRC_RESP	(uint8_t) 0x74

//#define KISS_TOGGLE_PTT			(uint8_t) 0x26
//#define KISS_RESTART_RESP		(uint8_t) 0x76

//#define KISS_CONTROL_VOLTAGE			(uint8_t) 0x27
//#define KISS_CONTROL_VOLTAGE_RESP		(uint8_t) 0x77

#define KISS_RETURN_IDLE		1


#endif /* KISS_COMMUNICATION_SERVICE_IDS_H_ */
