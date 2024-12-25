/*
 * software_version.h
 *
 *  Created on: Aug 7, 2023
 *      Author: mateusz
 */

#ifndef SOFTWARE_VERSION_H_
#define SOFTWARE_VERSION_H_

#include "build_datetime.h"

#define SW_VER "ED03"
#define SW_DATE BUILD_SWVERSTR
#define SW_KISS_PROTO	"C"

extern const char software_version_str[5];

#endif /* SOFTWARE_VERSION_H_ */
