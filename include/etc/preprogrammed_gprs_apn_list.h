/*
 * preprogrammed_gprs_apn_list.h
 *
 *  Created on: Aug 10, 2024
 *      Author: mateusz
 */

#ifndef ETC_PREPROGRAMMED_GPRS_APN_LIST_H_
#define ETC_PREPROGRAMMED_GPRS_APN_LIST_H_

#define MNC_POLSKA_PLUS_POLKOMTEL						1u

#define PREPROGRAMMED_GPRS_APN_LIST(ENTRY)							\
	/* MCC (country code), 	MNC (network code), 		APN name, 	user, 		password */				\
	ENTRY(MCC_POLSKA, 		MNC_POLSKA_PLUS_POLKOMTEL, 	"internet", "internet", "internet")				\



#endif /* ETC_PREPROGRAMMED_GPRS_APN_LIST_H_ */
