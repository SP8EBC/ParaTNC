#ifndef __GPIO_CONF_H
#define __GPIO_CONF_H

				 
#include <stm32f10x.h>

#define CNF_ANALOG		0
#define CNF_FLOATING		1
#define CNF_INPUT		2
#define CNF_RESERVED		3
 
#define CNF_GPPP		0
#define CNF_GPOD		1
#define CNF_AFPP		2
#define CNF_AFOD		3
 
#define MODE_INPUT		0
#define MODE_OUTPUT_2MHZ	1
#define MODE_OUTPUT_10MHZ	2
#define MODE_OUTPUT_50MHZ	3
 
#define ANALOG_MODE		((CNF_ANALOG << 2) | MODE_INPUT)
#define FLOATING_INPUT		((CNF_FLOATING << 2) | MODE_INPUT)
#define PUD_INPUT		((CNF_INPUT << 2) | MODE_INPUT)
 
#define GPPP_OUTPUT_2MHZ  	((CNF_GPPP << 2) | MODE_OUTPUT_2MHZ)
#define GPPP_OUTPUT_10MHZ 	((CNF_GPPP << 2) | MODE_OUTPUT_10MHZ)
#define GPPP_OUTPUT_50MHZ 	((CNF_GPPP << 2) | MODE_OUTPUT_50MHZ)
 
#define GPOD_OUTPUT_2MHZ  	((CNF_GPOD << 2) | MODE_OUTPUT_2MHZ)
#define GPOD_OUTPUT_10MHZ 	((CNF_GPOD << 2) | MODE_OUTPUT_10MHZ)
#define GPOD_OUTPUT_50MHZ 	((CNF_GPOD << 2) | MODE_OUTPUT_50MHZ)
 
#define AFPP_OUTPUT_2MHZ  	((CNF_AFPP << 2) | MODE_OUTPUT_2MHZ)
#define AFPP_OUTPUT_10MHZ 	((CNF_AFPP << 2) | MODE_OUTPUT_10MHZ)
#define AFPP_OUTPUT_50MHZ 	((CNF_AFPP << 2) | MODE_OUTPUT_50MHZ)
 
#define AFOD_OUTPUT_2MHZ  	((CNF_AFOD << 2) | MODE_OUTPUT_2MHZ)
#define AFOD_OUTPUT_10MHZ 	((CNF_AFOD << 2) | MODE_OUTPUT_10MHZ)
#define AFOD_OUTPUT_50MHZ 	((CNF_AFOD << 2) | MODE_OUTPUT_50MHZ)

void Configure_GPIO(GPIO_TypeDef* gpio, uint8_t pin, uint8_t conf);

#endif
