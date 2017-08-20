/**	
 * |----------------------------------------------------------------------
 * | Copyright (C) Tilen Majerle, 2014
 * | 
 * | This program is free software: you can redistribute it and/or modify
 * | it under the terms of the GNU General Public License as published by
 * | the Free Software Foundation, either version 3 of the License, or
 * | any later version.
 * |  
 * | This program is distributed in the hope that it will be useful,
 * | but WITHOUT ANY WARRANTY; without even the implied warranty of
 * | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * | GNU General Public License for more details.
 * | 
 * | You should have received a copy of the GNU General Public License
 * | along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * |----------------------------------------------------------------------
 */
#include "drivers/tm_stm32f1_onewire.h"
#include <stm32f10x.h>
#include <stm32f10x_gpio.h>
#include "diag/Trace.h"


int delay_5us = 0, global_delay_5us = 0;
int cntt = 0, temp2 = 0;

GPIO_InitTypeDef GPIO_input;
GPIO_InitTypeDef GPIO_output;

EXTI_InitTypeDef EXTI_InitStructure;
NVIC_InitTypeDef NVIC_I;


void TIM2_IRQHandler(void) {
	TIM2->SR &= ~(1<<0);

//	if ((GPIOC->ODR & GPIO_Pin_6)== GPIO_Pin_6)
//		GPIOC->BSRR |= (GPIO_Pin_6 << 16);
//	else
//		GPIOC->BSRR |= GPIO_Pin_6;

	delay_5us--;
	global_delay_5us--;
}

void TM_OneWire_Init(TM_OneWire_t* OneWireStruct, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) {
	/* Initialize delay if it was not already */
//	TM_DELAY_Init();

	
	/* Init GPIO pin */
//	TM_GPIO_Init(GPIOx, GPIO_Pin, TM_GPIO_Mode_OUT, TM_GPIO_OType_PP, TM_GPIO_PuPd_UP, TM_GPIO_Speed_Medium);

	GPIO_output.GPIO_Mode = GPIO_Mode_Out_OD;
//	GPIO_output.GPIO_OType = GPIO_OType_OD;
	GPIO_output.GPIO_Pin = GPIO_Pin;	
//	GPIO_output.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_output.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_input.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_input.GPIO_Pin = GPIO_Pin;	
//	GPIO_input.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_input.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOx, &GPIO_input);
	
	
//	NVIC_DisableIRQ(EXTI1_IRQn);

	/* Save settings */
	OneWireStruct->GPIOx = GPIOx;
	OneWireStruct->GPIO_Pin = GPIO_Pin;
	
	OneWireStruct->DataTRX = 0;
	
	OneWireStruct->Reset = 255;
	
	OneWireStruct->eStates = BUS_IDLE;
	OneWireStruct->eBusDir = IDL;
}

uint8_t TM_OneWire_Reset(TM_OneWire_t* OneWireStruct, char pool) {
	uint8_t i;
	
	if (OneWireStruct->eStates == BUS_IDLE && pool == 0) {
		/* Line low, and wait 500us */
		GPIO_Init(OneWireStruct->GPIOx, &GPIO_output);
		GPIO_ResetBits(OneWireStruct->GPIOx, OneWireStruct->GPIO_Pin);
		delay_5us = 100;
		OneWireStruct->eStates = RESET_LOW;
	//	ONEWIRE_DELAY(480);
	}
	
	else if (OneWireStruct->eStates == RESET_LOW && delay_5us <= 0) {
		/* Release line and wait for 70us */
		GPIO_Init(OneWireStruct->GPIOx, &GPIO_input);
		delay_5us = 20;
	//			cntt = 0;
	//	temp2 = 0;
	//	NVIC_EnableIRQ(EXTI1_IRQn);
		
//		SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource1);
//	
//		EXTI_InitStructure.EXTI_Line = 1 << EXTI_PinSource1;
//		EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
//		EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  
//		EXTI_InitStructure.EXTI_LineCmd = ENABLE;
//		EXTI_Init(&EXTI_InitStructure);

//		NVIC_I.NVIC_IRQChannel = EXTI1_IRQn;
//		NVIC_I.NVIC_IRQChannelPreemptionPriority = 6;
//		NVIC_I.NVIC_IRQChannelCmd = ENABLE;
//		NVIC_Init(&NVIC_I);
		

		OneWireStruct->eStates = RESET_WAIT_FOR_RESPONSE;
	//	ONEWIRE_DELAY(70);
	}
	
	else if (OneWireStruct->eStates == RESET_WAIT_FOR_RESPONSE && delay_5us <= 0) {	
		/* Check bit value */
		i = GPIO_ReadInputDataBit(OneWireStruct->GPIOx, OneWireStruct->GPIO_Pin);
		OneWireStruct->Reset = i;
		delay_5us = 100;
		OneWireStruct->eStates = RESET_FINISHING;
	}	
	else if (OneWireStruct->eStates == RESET_FINISHING && delay_5us <= 0) {
	/* Return value of presence pulse, 0 = OK, 1 = ERROR */
		OneWireStruct->eStates = RESET_COMPLETE;
//		trace_printf("OneWire: Reset_Complete");
	}

}

void TM_OneWire_WriteBit(TM_OneWire_t* OneWireStruct, uint8_t bit) {
	if (bit) {
		
		if (OneWireStruct->eStates == BUS_IDLE) {
			/* Set line low */
			GPIO_Init(OneWireStruct->GPIOx, &GPIO_output);
			GPIO_ResetBits(OneWireStruct->GPIOx, OneWireStruct->GPIO_Pin);
	//		ONEWIRE_DELAY(10);
			delay_5us = 2;
			OneWireStruct->eStates = WRITE_LOW;
		}
		
		else if (OneWireStruct->eStates == WRITE_LOW && delay_5us <= 0) {		
			/* Bit high */
			GPIO_Init(OneWireStruct->GPIOx, &GPIO_input);
			
			/* Wait for 55 us and release the line */
	//		ONEWIRE_DELAY(55);
			delay_5us = 15;
			OneWireStruct->eStates = WRITE_WAIT_FOR_RELASE;
		}
		
		else if (OneWireStruct->eStates == WRITE_WAIT_FOR_RELASE && delay_5us <= 0) {	
			GPIO_Init(OneWireStruct->GPIOx, &GPIO_input);
			OneWireStruct->eStates = WRITE_COMPLETE;			
		}
		
	} 
		else {
		
		if (OneWireStruct->eStates == BUS_IDLE) {	
			/* Set line low */
			GPIO_Init(OneWireStruct->GPIOx, &GPIO_output);
			GPIO_ResetBits(OneWireStruct->GPIOx, OneWireStruct->GPIO_Pin);
	//		ONEWIRE_DELAY(65);
			delay_5us = 15;
			OneWireStruct->eStates = WRITE_LOW;
		}
		
		else if (OneWireStruct->eStates == WRITE_LOW && delay_5us <= 0) {		
			/* Bit high */
			GPIO_Init(OneWireStruct->GPIOx, &GPIO_input);
			
			/* Wait for 5 us and release the line */
	//		ONEWIRE_DELAY(5);
			delay_5us = 4;
			OneWireStruct->eStates = WRITE_WAIT_FOR_RELASE;
		}
		
		else if (OneWireStruct->eStates == WRITE_WAIT_FOR_RELASE && delay_5us <= 0) {
			GPIO_Init(OneWireStruct->GPIOx, &GPIO_input);
			OneWireStruct->eStates = WRITE_COMPLETE;
		}
	}

}

uint8_t TM_OneWire_ReadBit(TM_OneWire_t* OneWireStruct) {
	uint8_t bit = 0;
	
	if (OneWireStruct->eStates == BUS_IDLE) {	
		/* Line low */
		GPIO_Init(OneWireStruct->GPIOx, &GPIO_output);
		GPIO_ResetBits(OneWireStruct->GPIOx, OneWireStruct->GPIO_Pin);
	//	ONEWIRE_DELAY(3);
		delay_5us = 0;
		OneWireStruct->eStates = 	READ_LOW;
	}
	
	else if (OneWireStruct->eStates == READ_LOW && delay_5us <= 0) {
		/* Release line */
		GPIO_Init(OneWireStruct->GPIOx, &GPIO_input);
	//	ONEWIRE_DELAY(10);
		delay_5us = 1;
		OneWireStruct->eStates = READ_WAIT_FOR_DATA;
	}
		
	else if (OneWireStruct->eStates == READ_WAIT_FOR_DATA && delay_5us <= 0) {
		/* Read line value */
//								GPIO_ToggleBits(GPIOB, GPIO_Pin_0);

		if (GPIO_ReadInputDataBit(OneWireStruct->GPIOx, OneWireStruct->GPIO_Pin)) {
			/* Bit is HIGH */
			bit = 1;
		}
		OneWireStruct->Bit = bit;
		
		OneWireStruct->DataTRX >>= 1;
		OneWireStruct->DataTRX |= (OneWireStruct->Bit << 7);
		OneWireStruct->BitCounter--;	
		
		/* Wait 50us to complete 60us period */
	//	ONEWIRE_DELAY(50);
		delay_5us = 10;
		OneWireStruct->eStates = 	READ_FINISHING;
	}
	
	else if (OneWireStruct->eStates == READ_FINISHING && delay_5us <= 0) {
		/* Return bit value */
		OneWireStruct->eStates = READ_COMPLETE;
		return bit;
	}
}



void TM_OneWire_WriteByte(TM_OneWire_t* OneWireStruct, uint8_t byte, char pool) {
	
	if (OneWireStruct->eBusDir == IDL && pool == 0) {
		OneWireStruct->DataTRX = byte;
		OneWireStruct->eBusDir = WRI;
		OneWireStruct->eStates = BUS_IDLE;
		OneWireStruct->BitCounter = 8;
	}
	
	else if (OneWireStruct->eBusDir == WRI && OneWireStruct->eStates == BUS_IDLE && pool == 1) {
		/* Write 8 bits */
		OneWireStruct->BitCounter--;
		if (OneWireStruct->BitCounter >= 0) {
			/* LSB bit is first */
			TM_OneWire_WriteBit(OneWireStruct, OneWireStruct->DataTRX & 0x01);
		}
		else
			OneWireStruct->eBusDir = IDL;
	}
	
	else if (OneWireStruct->eBusDir == WRI && OneWireStruct->eStates == WRITE_COMPLETE && pool == 1) {
		OneWireStruct->DataTRX >>= 1;
		OneWireStruct->eStates = BUS_IDLE;
		
	}
	
	else if (OneWireStruct->eBusDir == WRI && OneWireStruct->eStates != BUS_IDLE && pool == 1) {
		TM_OneWire_WriteBit(OneWireStruct, OneWireStruct->DataTRX & 0x01);		
	}
	else;
	
}




uint8_t TM_OneWire_ReadByte(TM_OneWire_t* OneWireStruct, char pool) {
	
	if (OneWireStruct->eBusDir == IDL && pool == 0) {	
		OneWireStruct->DataTRX = 0;
		OneWireStruct->eBusDir = RD;
		OneWireStruct->BitCounter = 8;
	}
	
	else if (OneWireStruct->eBusDir == RD && OneWireStruct->eStates == READ_COMPLETE && pool == 1) {
		if (OneWireStruct->BitCounter > 0) {
			OneWireStruct->eStates = BUS_IDLE;
			TM_OneWire_ReadBit(OneWireStruct);
		}
		else
						OneWireStruct->eBusDir = IDL;
	}
	
	else if (OneWireStruct->eBusDir == RD && OneWireStruct->eStates != READ_COMPLETE && pool == 1) {
		TM_OneWire_ReadBit(OneWireStruct);
	}	
		else;
//	return byte;
}


uint8_t TM_OneWire_CRC8(uint8_t *addr, uint8_t len) {
	uint8_t crc = 0, inbyte, i, mix;
	
	while (len--) {
		inbyte = *addr++;
		for (i = 8; i; i--) {
			mix = (crc ^ inbyte) & 0x01;
			crc >>= 1;
			if (mix) {
				crc ^= 0x8C;
			}
			inbyte >>= 1;
		}
	}
	
	/* Return calculated CRC */
	return crc;
}

void Dallas_Pool(Dallas* ds, TM_OneWire_t* onewire) {
	uint8_t crc = 0;
	char temp1, temp2, sign;
	unsigned temp3;
	
	if (ds->command == CONVERSION && onewire->eStates == RESET_COMPLETE && ds->bytes == 0) {
			TM_OneWire_WriteByte(onewire, 0xCC, 0);
			global_delay_5us = 300;
			ds->command = CONVERSION_SKIP;
	}
	
	else if (ds->command == CONVERSION_SKIP && onewire->eBusDir == IDL && onewire->eStates == BUS_IDLE && ds->bytes == 0 && global_delay_5us <= 0) {
			TM_OneWire_WriteByte(onewire, 0x44, 0);
			global_delay_5us = 800000;
			ds->command = CONVERSION_RESET;
	}
	
	else if (ds->command == CONVERSION_RESET && onewire->eBusDir == IDL && onewire->eStates == BUS_IDLE && ds->bytes == 0 && global_delay_5us <= 0) {
			TM_OneWire_Reset(onewire, 0);
			global_delay_5us = 300;
			ds->command = READ_SCR;
	}
	
	else if (ds->command == READ_SCR && onewire->eStates == RESET_COMPLETE && ds->bytes == 0 && global_delay_5us <= 0) {
			TM_OneWire_WriteByte(onewire, 0xCC, 0);
			ds->command = READ_SCR_SKIP;
			global_delay_5us = 300;
	}
	
	else if (ds->command == READ_SCR_SKIP && onewire->eStates == BUS_IDLE && onewire->eBusDir == IDL && ds->bytes == 0 && global_delay_5us <= 0) {	
			TM_OneWire_WriteByte(onewire, 0xBE, 0);
			ds->command = READ_SCR_WRITE;
			global_delay_5us = 300;
	}
	
	else if (ds->command == READ_SCR_WRITE && onewire->eBusDir == IDL && onewire->eStates == BUS_IDLE && global_delay_5us <= 0) {
			TM_OneWire_ReadByte(onewire, 0);
			ds->command = READ_SCR_READ;
			global_delay_5us = 300;
	}
	
	else if (ds->command == READ_SCR_READ && onewire->eStates == READ_COMPLETE && onewire->eBusDir == IDL && global_delay_5us <= 0) {
			ds->scratchpad[ds->bytes] = onewire->DataTRX;
			TM_OneWire_ReadByte(onewire, 0);
			global_delay_5us = 200;
			if (ds->bytes++ > 8) {
				ds->command = I;
				ds->current_crc = TM_OneWire_CRC8(ds->scratchpad, 8);
				if (ds->current_crc == ds->scratchpad[8]) {
					temp3 = ds->scratchpad[0] | (ds->scratchpad[1] << 8);
					temp1 = (0x7F0 & temp3) >> 4;
					temp2 = 0xF & temp3;
					sign = (temp3 & 0xF000) ? -1 : 1;
					if (sign == 1)
						ds->temperature = (float)temp1 + temp2 * 0.0625f;
					else
						ds->temperature = -1.0f * (128.0f - (float)temp1 - (float)temp2 * 0.0625f);
				}
			}
	}
	
	else if (ds->command == READ_SCR_SKIP && onewire->eStates == BUS_IDLE && onewire->eBusDir == IDL && ds->bytes == 0 && global_delay_5us <= 0) {	
			TM_OneWire_WriteByte(onewire, 0x44, 0);
			ds->command = I;
	}
	///////	
	else if (ds->command == READ_ROM && onewire->eStates == RESET_COMPLETE && ds->bytes == 0) {
			TM_OneWire_WriteByte(onewire, 0x33, 0);
			ds->command = READ_ROM_READ;
			global_delay_5us = 200;
			
	}
	
	else if (ds->command == READ_ROM_READ && onewire->eBusDir == IDL && onewire->eStates == BUS_IDLE && global_delay_5us <= 0) {
			global_delay_5us = 200;
			TM_OneWire_ReadByte(onewire, 0);	
	}
	
	else if (ds->command == READ_ROM_READ && onewire->eStates == READ_COMPLETE && onewire->eBusDir == IDL && global_delay_5us <= 0) {
			ds->ROM[ds->bytes] = onewire->DataTRX;
			TM_OneWire_ReadByte(onewire, 0);
			global_delay_5us = 200;
			if (ds->bytes++ > 8) {
				ds->command = I;
				ds->current_crc = TM_OneWire_CRC8(ds->ROM, 7);

			}
	}
}

void Dallas_Init(Dallas* ds) {
	ds->bytes = 0;
	ds->temperature_valid = 0;
	ds->temperature = 0.0f;
	ds->current_crc = 0;
	memset(ds->scratchpad, 0x00, 9);
	memset(ds->ROM, 0x00, 8);
}

