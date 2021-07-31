/*
 * dma_helper_functions.h
 *
 *  Created on: 28.12.2019
 *      Author: mateusz
 */

#ifndef INCLUDE_DRIVERS_DMA_HELPER_FUNCTIONS_H_
#define INCLUDE_DRIVERS_DMA_HELPER_FUNCTIONS_H_

#ifdef STM32F10X_MD_VL
#include <stm32f10x_dma.h>
void dma_helper_start_ch7(DMA_InitTypeDef* DMA_InitStruct);
#endif
#ifdef STM32L471xx
#include <stm32l4xx.h>
#include <stm32l4xx_ll_dma.h>
void dma_helper_start_ch7(LL_DMA_InitTypeDef* DMA_InitStruct);
#endif



#endif /* INCLUDE_DRIVERS_DMA_HELPER_FUNCTIONS_H_ */
