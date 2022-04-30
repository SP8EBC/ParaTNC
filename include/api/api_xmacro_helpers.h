/*
 * api_xmacro_helpers.h
 *
 *  Created on: Apr 25, 2022
 *      Author: mateusz
 */

#ifndef API_XMACRO_HELPERS_H_
#define API_XMACRO_HELPERS_H_

#include <stdio.h>
#include <string.h>

#define BEGIN	\
	memset(OUT, 0x00, sizeof(OUT));		\
	LN = sprintf(OUT + LN, "{\r\n");	\


#define PRINT_32INT(integer, name)	LN += sprintf(OUT + LN, "\"" #name "\":%ld,", integer);
#define PRINT_16INT(integer, name)	LN += sprintf(OUT + LN, "\"" #name "\":%d,", integer);
#define PRINT_STRING(string, name)	LN += sprintf(OUT + LN, "\"" #name "\":\"%s\",", string);
#define END	LN += sprintf(OUT + LN - 1, "}\r\n");


#define PRINT_ALL_STATUS	\
	ENTRIES_32INT_STATUS(PRINT_32INT);			\
	ENTRIES_16INT_STATUS(PRINT_16INT);			\
	ENTRIES_STRING(PRINT_STRING);				\

#define PRINT_ALL_MEASUREMENTS	\
	ENTRIES_16INT_WEATHER(PRINT_16INT)			\

#endif /* API_XMACRO_HELPERS_H_ */
