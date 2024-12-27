/*
 * software_version.c
 *
 * Software version was declared previously in main.h as a #define
 *
 *  Created on: Aug 7, 2023
 *      Author: mateusz
 */

#include "software_version.h"
#include "build_datetime.h"

const char software_version_str[5] = SW_VER;
const char software_version_date[9] = BUILD_SWVERSTR;
