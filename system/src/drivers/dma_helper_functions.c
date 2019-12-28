/*
 * dma_helper_functions.c
 *
 *  Created on: 28.12.2019
 *      Author: mateusz
 */


#include <stm32f10x_dma.h>


void dma_helper_start_ch7(DMA_InitTypeDef* DMA_InitStruct) {
	DMA_Init(DMA1_Channel7, DMA_InitStruct);
	DMA1_Channel7->CCR |= DMA_CCR7_EN;
	DMA1_Channel7->CCR |= DMA_CCR7_TCIE;
}
