/*
 * dma_helper_functions.c
 *
 *  Created on: 28.12.2019
 *      Author: mateusz
 */

#include "./drivers/dma_helper_functions.h"
#include "station_config_target_hw.h"
#ifdef STM32F10X_MD_VL

void dma_helper_start_ch7(DMA_InitTypeDef* DMA_InitStruct) {
	DMA_DeInit(DMA1_Channel7);
	DMA_Init(DMA1_Channel7, DMA_InitStruct);
	DMA1_Channel7->CCR |= DMA_CCR7_EN;
	DMA1_Channel7->CCR |= DMA_CCR7_TCIE;
}

#endif

#ifdef STM32L471xx

void dma_helper_start_ch7(LL_DMA_InitTypeDef* DMA_InitStruct) {
	//DMA_DeInit(DMA1_Channel7);
	LL_DMA_DeInit(DMA1, LL_DMA_CHANNEL_5);
	LL_DMA_Init(DMA1, LL_DMA_CHANNEL_5, DMA_InitStruct);

	LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_5);

	LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_5);

	//DMA1_Channel7->CCR |= DMA_CCR7_EN;
	//DMA1_Channel7->CCR |= DMA_CCR7_TCIE;
}

#endif
