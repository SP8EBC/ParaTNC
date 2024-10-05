/**
 *
 * WARNING! This file has been highly modified by // ML during ParaTNC porting
 * from STM32F100 to STM32L4xx family
 *
  ******************************************************************************
  * @file    system_stm32l4xx.c
  * @author  MCD Application Team
  * @brief   CMSIS Cortex-M4 Device Peripheral Access Layer System Source File
  *
  *   This file provides two functions and one global variable to be called from
  *   user application:
  *      - SystemInit(): This function is called at startup just after reset and
  *                      before branch to main program. This call is made inside
  *                      the "startup_stm32l4xx.s" file.
  *
  *      - SystemCoreClock variable: Contains the core clock (HCLK), it can be used
  *                                  by the user application to setup the SysTick
  *                                  timer or configure other parameters.
  *
  *      - SystemCoreClockUpdate(): Updates the variable SystemCoreClock and must
  *                                 be called whenever the core clock is changed
  *                                 during program execution.
  *
  *   After each device reset the MSI (4 MHz) is used as system clock source.
  *   Then SystemInit() function is called, in "startup_stm32l4xx.s" file, to
  *   configure the system clock before to branch to main program.
  *
  *   This file configures the system clock as follows:
  *=============================================================================
  *-----------------------------------------------------------------------------
  *        System Clock source                    | PLL
  *-----------------------------------------------------------------------------
  *        SYSCLK(Hz)                             | 48000000
  *-----------------------------------------------------------------------------
  *        HCLK(Hz)                               | 48000000
  *-----------------------------------------------------------------------------
  *        AHB Prescaler                          | 1
  *-----------------------------------------------------------------------------
  *        APB1 Prescaler                         | 2
  *-----------------------------------------------------------------------------
  *        APB2 Prescaler                         | 2
  *-----------------------------------------------------------------------------
  *        PLL_M                                  | 1
  *-----------------------------------------------------------------------------
  *        PLL_N                                  | 12
  *-----------------------------------------------------------------------------
  *        PLL_P                                  | 7
  *-----------------------------------------------------------------------------
  *        PLL_Q                                  | 2
  *-----------------------------------------------------------------------------
  *        PLL_R                                  | 0
  *-----------------------------------------------------------------------------
  *        PLLSAI1_P                              | NA
  *-----------------------------------------------------------------------------
  *        PLLSAI1_Q                              | NA
  *-----------------------------------------------------------------------------
  *        PLLSAI1_R                              | NA
  *-----------------------------------------------------------------------------
  *        PLLSAI2_P                              | NA
  *-----------------------------------------------------------------------------
  *        PLLSAI2_Q                              | NA
  *-----------------------------------------------------------------------------
  *        PLLSAI2_R                              | NA
  *-----------------------------------------------------------------------------
  *        Require 48MHz for USB OTG FS,          | Disabled
  *        SDIO and RNG clock                     |
  *-----------------------------------------------------------------------------
  *=============================================================================
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Apache License, Version 2.0,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/Apache-2.0
  *
  ******************************************************************************
  */

/** @addtogroup CMSIS
  * @{
  */

/** @addtogroup stm32l4xx_system
  * @{
  */

/** @addtogroup STM32L4xx_System_Private_Includes
  * @{
  */

#include "stm32l4xx.h"
#include "stm32l4xx_ll_rcc.h"
#include "stm32l4xx_hal_rcc.h"
#include "stm32l4xx_hal_flash.h"
#include "stm32l4xx_hal_pwr_ex.h"

/**
  * @}
  */

/** @addtogroup STM32L4xx_System_Private_TypesDefinitions
  * @{
  */

/**
  * @}
  */

/** @addtogroup STM32L4xx_System_Private_Defines
  * @{
  */

#define SYSTEM_CLOCK_RTC_CLOCK_TIMEOUT 0x199999


#if !defined  (HSE_VALUE)
  #define HSE_VALUE    8000000U  /*!< Value of the External oscillator in Hz */
#endif /* HSE_VALUE */

#if !defined  (MSI_VALUE)
  #define MSI_VALUE    4000000U  /*!< Value of the Internal oscillator in Hz*/
#endif /* MSI_VALUE */

#if !defined  (HSI_VALUE)
  #define HSI_VALUE    16000000U /*!< Value of the Internal oscillator in Hz*/
#endif /* HSI_VALUE */

/* Note: Following vector table addresses must be defined in line with linker
         configuration. */
/*!< Uncomment the following line if you need to relocate the vector table
     anywhere in Flash or Sram, else the vector table is kept at the automatic
     remap of boot address selected */
#define USER_VECT_TAB_ADDRESS

#if defined(USER_VECT_TAB_ADDRESS)
/*!< Uncomment the following line if you need to relocate your vector Table
     in Sram else user remap will be done in Flash. */
/* #define VECT_TAB_SRAM */

#if defined(VECT_TAB_SRAM)
#define VECT_TAB_BASE_ADDRESS   SRAM1_BASE      /*!< Vector Table base address field.
                                                     This value must be a multiple of 0x200. */
#define VECT_TAB_OFFSET         0x00000000U     /*!< Vector Table base offset field.
                                                     This value must be a multiple of 0x200. */
#else
#define VECT_TAB_BASE_ADDRESS   0x0800C000U     /*!< Vector Table base address field.
                                                     This value must be a multiple of 0x200. */
#define VECT_TAB_OFFSET         0x00000000U     /*!< Vector Table base offset field.
                                                     This value must be a multiple of 0x200. */
#endif /* VECT_TAB_SRAM */
#endif /* USER_VECT_TAB_ADDRESS */

/******************************************************************************/
/**
  * @}
  */

/** @addtogroup STM32L4xx_System_Private_Macros
  * @{
  */

/**
  * @}
  */

/** @addtogroup STM32L4xx_System_Private_Variables
  * @{
  */
  /* The SystemCoreClock variable is updated in three ways:
      1) by calling CMSIS function SystemCoreClockUpdate()
      2) by calling HAL API function HAL_RCC_GetHCLKFreq()
      3) each time HAL_RCC_ClockConfig() is called to configure the system clock frequency
         Note: If you use this function to configure the system clock; then there
               is no need to call the 2 first functions listed above, since SystemCoreClock
               variable is updated automatically.
  */
  uint32_t SystemCoreClock = 4000000U;

  uint32_t SystemRtcHasFailed = 0;

  volatile uint32_t timeout_counter = 0;

  const uint8_t  AHBPrescTable[16] = {0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 1U, 2U, 3U, 4U, 6U, 7U, 8U, 9U};
  const uint8_t  APBPrescTable[8] =  {0U, 0U, 0U, 0U, 1U, 2U, 3U, 4U};
  const uint32_t MSIRangeTable[12] = {100000U,   200000U,   400000U,   800000U,  1000000U,  2000000U, \
                                      4000000U, 8000000U, 16000000U, 24000000U, 32000000U, 48000000U};
/**
  * @}
  */

/** @addtogroup STM32L4xx_System_Private_FunctionPrototypes
  * @{
  */

/**
  * @}
  */

/** @addtogroup STM32L4xx_System_Private_Functions
  * @{
  */

/**
  * @brief  Setup the microcontroller system.
  * @retval None
  */

void SystemInit(void)
{
#if defined(USER_VECT_TAB_ADDRESS)
  /* Configure the Vector Table location -------------------------------------*/
  SCB->VTOR = VECT_TAB_BASE_ADDRESS | VECT_TAB_OFFSET;
#endif

  /* FPU settings ------------------------------------------------------------*/
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
  SCB->CPACR |= ((3UL << 20U)|(3UL << 22U));  /* set CP10 and CP11 Full Access */
#endif

  /* Enable usage, bus and memory faults */
  SCB->SHCSR |= SCB_SHCSR_USGFAULTACT_Msk;
  SCB->SHCSR |= SCB_SHCSR_BUSFAULTENA_Msk;
  SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk;

  /* Reset the RCC clock configuration to the default reset state ------------*/
  /* Set MSION bit */
  RCC->CR |= RCC_CR_MSION;

  /* Reset CFGR register */
  RCC->CFGR = 0x00000000U;

  /* Reset HSEON, CSSON , HSION, and PLLON bits */
  RCC->CR &= 0xEAF6FFFFU;

  /* Reset PLLCFGR register */
  RCC->PLLCFGR = 0x00001000U;

  /* Reset HSEBYP bit */
  RCC->CR &= 0xFFFBFFFFU;

  /* Disable all interrupts */
  RCC->CIER = 0x00000000U;
}

/**
  * @brief  Update SystemCoreClock variable according to Clock Register Values.
  *         The SystemCoreClock variable contains the core clock (HCLK), it can
  *         be used by the user application to setup the SysTick timer or configure
  *         other parameters.
  *
  * @note   Each time the core clock (HCLK) changes, this function must be called
  *         to update SystemCoreClock variable value. Otherwise, any configuration
  *         based on this variable will be incorrect.
  *
  * @note   - The system frequency computed by this function is not the real
  *           frequency in the chip. It is calculated based on the predefined
  *           constant and the selected clock source:
  *
  *           - If SYSCLK source is MSI, SystemCoreClock will contain the MSI_VALUE(*)
  *
  *           - If SYSCLK source is HSI, SystemCoreClock will contain the HSI_VALUE(**)
  *
  *           - If SYSCLK source is HSE, SystemCoreClock will contain the HSE_VALUE(***)
  *
  *           - If SYSCLK source is PLL, SystemCoreClock will contain the HSE_VALUE(***)
  *             or HSI_VALUE(*) or MSI_VALUE(*) multiplied/divided by the PLL factors.
  *
  *         (*) MSI_VALUE is a constant defined in stm32l4xx_hal.h file (default value
  *             4 MHz) but the real value may vary depending on the variations
  *             in voltage and temperature.
  *
  *         (**) HSI_VALUE is a constant defined in stm32l4xx_hal.h file (default value
  *              16 MHz) but the real value may vary depending on the variations
  *              in voltage and temperature.
  *
  *         (***) HSE_VALUE is a constant defined in stm32l4xx_hal.h file (default value
  *              8 MHz), user has to ensure that HSE_VALUE is same as the real
  *              frequency of the crystal used. Otherwise, this function may
  *              have wrong result.
  *
  *         - The result of this function could be not correct when using fractional
  *           value for HSE crystal.
  *
  * @retval None
  */
void system_clock_update_l4(void)
{
  uint32_t tmp, msirange, pllvco, pllsource, pllm, pllr;

  /* Get MSI Range frequency--------------------------------------------------*/
  if ((RCC->CR & RCC_CR_MSIRGSEL) == 0U)
  { /* MSISRANGE from RCC_CSR applies */
    msirange = (RCC->CSR & RCC_CSR_MSISRANGE) >> 8U;
  }
  else
  { /* MSIRANGE from RCC_CR applies */
    msirange = (RCC->CR & RCC_CR_MSIRANGE) >> 4U;
  }
  /*MSI frequency range in HZ*/
  msirange = MSIRangeTable[msirange];

  /* Get SYSCLK source -------------------------------------------------------*/
  switch (RCC->CFGR & RCC_CFGR_SWS)
  {
    case 0x00:  /* MSI used as system clock source */
      SystemCoreClock = msirange;
      break;

    case 0x04:  /* HSI used as system clock source */
      SystemCoreClock = HSI_VALUE;
      break;

    case 0x08:  /* HSE used as system clock source */
      SystemCoreClock = HSE_VALUE;
      break;

    case 0x0C:  /* PLL used as system clock  source */
      /* PLL_VCO = (HSE_VALUE or HSI_VALUE or MSI_VALUE/ PLLM) * PLLN
         SYSCLK = PLL_VCO / PLLR
         */
      pllsource = (RCC->PLLCFGR & RCC_PLLCFGR_PLLSRC);
      pllm = ((RCC->PLLCFGR & RCC_PLLCFGR_PLLM) >> 4U) + 1U ;

      switch (pllsource)
      {
        case 0x02:  /* HSI used as PLL clock source */
          pllvco = (HSI_VALUE / pllm);
          break;

        case 0x03:  /* HSE used as PLL clock source */
          pllvco = (HSE_VALUE / pllm);
          break;

        default:    /* MSI used as PLL clock source */
          pllvco = (msirange / pllm);
          break;
      }
      pllvco = pllvco * ((RCC->PLLCFGR & RCC_PLLCFGR_PLLN) >> 8U);
      pllr = (((RCC->PLLCFGR & RCC_PLLCFGR_PLLR) >> 25U) + 1U) * 2U;
      SystemCoreClock = pllvco/pllr;
      break;

    default:
      SystemCoreClock = msirange;
      break;
  }
  /* Compute HCLK clock frequency --------------------------------------------*/
  /* Get HCLK prescaler */
  tmp = AHBPrescTable[((RCC->CFGR & RCC_CFGR_HPRE) >> 4U)];
  /* HCLK clock frequency */
  SystemCoreClock >>= tmp;
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
int system_clock_configure_l4(void)
{
	/**
	 * 	Bits 26:25 PLLR[1:0]: Main PLL division factor for PLLCLK (system clock)
	 *	Set and cleared by software to control the frequency of the main PLL output clock PLLCLK.
	 *	This output can be selected as system clock. These bits can be written only if PLL is
	 *	disabled.
	 *	PLLCLK output clock frequency = VCO frequency / PLLR with PLLR = 2, 4, 6, or 8
	 *	00: PLLR = 2
	 *	01: PLLR = 4
	 *	10: PLLR = 6
	 *	11: PLLR = 8
	 *	Caution:
	 *	The software has to set these bits correctly not to exceed 80 MHz on
	 *	this domain.
	 *
	 *
	 */

	/**
	 * 	Bits 7:4 HPRE[3:0]: AHB prescaler
		Set and cleared by software to control the division factor of the AHB clock.
		Caution:
		Depending on the device voltage range, the software has to set
		correctly these bits to ensure that the system frequency does not
		exceed the maximum allowed frequency (for more details please refer to
		Section 5.1.8: Dynamic voltage scaling management). After a write
		operation to these bits and before decreasing the voltage range, this
		register must be read to be sure that the new value has been taken into
		account.
		0xxx: SYSCLK not divided
		1000: SYSCLK divided by 2
	 *
	 *
	 *
	 *
	 */

  /** Configure LSE Drive Capability
  */
  //HAL_PWR_EnableBkUpAccess();
  //__HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

#ifdef HI_SPEED
  // set the flash latency
  FLASH->ACR |= FLASH_ACR_LATENCY_3WS;
#else
  // set the flash latency
  FLASH->ACR |= FLASH_ACR_LATENCY_2WS;
#endif

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */

  // select MSI as system clock cource
  // RCC_CFGR_SW_MSI
  RCC->CFGR &= (0xFFFFFFFF ^ RCC_CFGR_SW_Msk);

#ifdef HI_SPEED
  // set APB1 prescaler - HCLK divided by 2
  RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;

  // set APB2 prescaler - HCLK divided by 2
  RCC->CFGR |= RCC_CFGR_PPRE2_DIV2;
#endif

  // turn on high speed external quartz oscilator
  RCC->CR |= RCC_CR_HSEON;

  // turn of the PLL1 before any configuration change
  RCC->CR &= (0xFFFFFFFF ^ RCC_CR_PLLON);

  // be sure that PLL is not running
  while ((RCC->CR & RCC_CR_PLLRDY) != 0);

  // reset PLLCFGR register
  RCC->PLLCFGR = 0;

  // set the clock source for PLL
  RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC_HSE;

#ifndef HI_SPEED
  // R division factor for PLL to /2 (DIV2)
  RCC->PLLCFGR |= RCC_PLLCFGR_PLLR_0;	//(default reset value of 00: PLLR = 2)
#endif

  // Q divistion factor for PLL to /2 (DIV2)
  RCC->PLLCFGR &= (0xFFFFFFFF ^ (RCC_PLLCFGR_PLLQ_Msk));

  // P division factor for PLL to /7 (DIV7)
  RCC->PLLCFGR &= (0xFFFFFFFF ^ (RCC_PLLCFGR_PLLP_Msk));

  // M division factor to 1
  RCC->PLLCFGR &= (0xFFFFFFFF ^ (RCC_PLLCFGR_PLLM_Msk));

  // N multiplication factor to 12
  RCC->PLLCFGR |= (12 << RCC_PLLCFGR_PLLN_Pos);

  // turn on the PLL
  RCC->CR |= RCC_CR_PLLON;

  // wait for PLL to startup and lock
  while ((RCC->CR & RCC_CR_PLLRDY) == 0);

  // turn on all PLL outputs
  RCC->PLLCFGR |= RCC_PLLCFGR_PLLREN;
  RCC->PLLCFGR |= RCC_PLLCFGR_PLLPEN;
  RCC->PLLCFGR |= RCC_PLLCFGR_PLLQEN;

  // turn on LSI (required by IWDG)
  RCC->CSR |= RCC_CSR_LSION;

  // select PLL as a system clock
  RCC->CFGR |= RCC_CFGR_SW_PLL;

  // wait for the clock to switch
  while ((RCC->CFGR & RCC_CFGR_SWS_PLL) != RCC_CFGR_SWS_PLL);

  // configure clock sources for some peripherals
  RCC->CCIPR |= (RCC_CCIPR_ADCSEL | RCC_CCIPR_CLK48SEL_1);		// system clock selected for ADC

  // main core frequency should be now set to 48MHz
  return 0;
}

void system_clock_start_rtc_l4(void) {

	volatile uint32_t timeout_counter = 0;

	if ((RCC->BDCR & RCC_BDCR_LSERDY) == 0) {
		SystemRtcHasFailed = 1;

		return;
	}

	// starting RTC
	RCC->BDCR |= RCC_BDCR_RTCEN;

	// enable write access to RTC registers by writing two magic words
	RTC->WPR = 0xCA;
	RTC->WPR = 0x53;

	// enter the clock set mode
	RTC->ISR |= RTC_ISR_INIT;

	// wait for going into clock set mode
	while((RTC->ISR & RTC_ISR_INITF) == 0) {
		if (timeout_counter++ > SYSTEM_CLOCK_RTC_CLOCK_TIMEOUT) {
			SystemRtcHasFailed = 1;

			return;
		}
	}

	// set date
	RTC->DR = 0x0024E714;

	// set time
	RTC->TR = 0x00182044;

	// exit RTC set mode
	RTC->ISR &= (0xFFFFFFFF ^ RTC_ISR_INIT);

	// disable wakeup interrupt and wakeup interrupt
	RTC->CR = 0;

	// wait for wakeup timer to disable
	while((RTC->ISR & RTC_ISR_WUTWF) == 0);

	// set the source clock for RTC wakeup as CK_SPRE
	RTC->CR |= RTC_CR_WUCKSEL_2;
}

int system_clock_configure_rtc_l4(void) {

	int retval = 0;

	// check if LSE is working now
	uint8_t lse_is_working = ((RCC->BDCR & RCC_BDCR_LSERDY) > 0) ? 1 : 0;

	// check the current clock source for RTC
	uint8_t valid_rtc_clk_source = ((RCC->BDCR & RCC_BDCR_RTCSEL) == RCC_BDCR_RTCSEL_0) ? 1 : 0;

	uint8_t rtc_started = ((RCC->BDCR & RCC_BDCR_RTCEN) > 0) ? 1 : 0;

	// enable access to backup domain
	PWR->CR1 |= PWR_CR1_DBP;

	// if LSE is not working reinitialize everything
	if (lse_is_working == 0 || valid_rtc_clk_source == 0 || rtc_started == 0) {

		// reset backup domain
		RCC->BDCR |= RCC_BDCR_BDRST;

		// wait for the reset
		while (timeout_counter < SYSTEM_CLOCK_RTC_CLOCK_TIMEOUT) {
			// wait a little bit for a reset to be done
			timeout_counter++;
		}

		// reset timeout timer
		timeout_counter = 0;

		// but clear reset flag before
		RCC->BDCR &= (0xFFFFFFFF ^ RCC_BDCR_BDRST);

		// set the clock source for RTC clock to LSE
		RCC->BDCR |= RCC_BDCR_RTCSEL_0;

		// set LSE quartz driving to medium-high
		RCC->BDCR |= RCC_BDCR_LSEDRV_1;

		// turn on LSE
		RCC->BDCR |= RCC_BDCR_LSEON;

		// wait for LSE to start
		while((RCC->BDCR & RCC_BDCR_LSERDY) == 0) {
			if (timeout_counter++ > SYSTEM_CLOCK_RTC_CLOCK_TIMEOUT) {
				retval = -1;

				SystemRtcHasFailed = 1;

				break;
			}
		}

		if (SystemRtcHasFailed == 0) {
			// starting and configuring the RTC itself
			system_clock_start_rtc_l4();
		}
	}

	// disable access do backup domain
	PWR->CR1 &= (0xFFFFFFFF ^ PWR_CR1_DBP);

	return retval;
}

void system_clock_configure_auto_wakeup_l4(uint16_t seconds) {

	// enable access to backup domain
	PWR->CR1 |= PWR_CR1_DBP;

	// check if RTC is working
	if ((RCC->BDCR & RCC_BDCR_RTCEN) == 0) {
		system_clock_start_rtc_l4();
	}

	// check if RTC initialization has succeded
	if (SystemRtcHasFailed == 1) {
		return;
	}

	// enable write access to RTC registers by writing two magic words
	RTC->WPR = 0xCA;
	RTC->WPR = 0x53;

	// disable wakeup timer
	RTC->CR &= (0xFFFFFFFF ^ RTC_CR_WUTE);

	// wait for wakeup timer to disable
	while((RTC->ISR & RTC_ISR_WUTWF) == 0);

	// clear wakeup flag
	RTC->ISR &= (0xFFFFFFFF ^ RTC_ISR_WUTF_Msk);

	// set auto wakeup timer
	RTC->WUTR = seconds;

	// start wakeup timer once again
	RTC->CR |= RTC_CR_WUTE;

	// enabling wakeup interrupt
	RTC->CR |= RTC_CR_WUTIE;

	// enable 20th EXTI Line (RTC wakeup timer)
	EXTI->IMR1 |= EXTI_IMR1_IM20;

	// set 20th EXTI line to rising trigger
	EXTI->RTSR1 |= EXTI_RTSR1_RT20;

	// by enabling this all pending interrupt will wake up cpu from low-power mode, even from those disabled in NVIC
	SCB->SCR |= SCB_SCR_SEVONPEND_Msk;

	// enable wakeup interrupt
	NVIC_EnableIRQ(RTC_WKUP_IRQn);

	// disable access do backup domain
	PWR->CR1 &= (0xFFFFFFFF ^ PWR_CR1_DBP);
}

int system_is_rtc_ok(void) {
	int result = 1;

	// check if LSE is working now
	uint8_t lse_is_working = ((RCC->BDCR & RCC_BDCR_LSERDY) > 0) ? 1 : 0;

	if (SystemRtcHasFailed == 1) {
		result = 0;
	}

	if ((RCC->BDCR & RCC_BDCR_RTCEN) == 0) {
		result = 0;
	}

	if (lse_is_working == 0) {
		result = 0;
	}

	return result;
}

uint32_t system_get_rtc_date(void) {
	return RTC->DR;
}

uint32_t system_get_rtx_time(void) {
	return RTC->TR;
}


/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
