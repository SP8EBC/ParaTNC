#include "./kiss_communication/diagnostics_routines/routine_5254_set_datetime.h"

#include "tm/tm_stm32_rtc.h"
#include "stm32l4xx_ll_rtc.h"

/// ==================================================================================================
///	LOCAL DEFINITIONS
/// ==================================================================================================

/// ==================================================================================================
///	LOCAL DATA TYPES
/// ==================================================================================================

/// ==================================================================================================
///	LOCAL VARIABLES
/// ==================================================================================================

static LL_RTC_TimeTypeDef routine_5254_rtc_time;

static LL_RTC_DateTypeDef routine_5254_rtc_date;

static uint16_t routine_5254_result = 0;

/// ==================================================================================================
///	GLOBAL VARIABLES
/// ==================================================================================================

/// ==================================================================================================
///	LOCAL FUNCTIONS
/// ==================================================================================================

/// ==================================================================================================
///	LOCAL FUNCTIONS
/// ==================================================================================================

static int routine_5254_get_weekday_from_date(const LL_RTC_DateTypeDef * in)
{

    int day = __LL_RTC_CONVERT_BIN2BCD(in->Day); 
    int month = __LL_RTC_CONVERT_BIN2BCD(in->Month); 
    int year = __LL_RTC_CONVERT_BIN2BCD(in->Year);

       const int out =(                                            \
          day                                                      \
        + ((153 * (month + 12 * ((14 - month) / 12) - 3) + 2) / 5) \
        + (365 * (year + 4800 - ((14 - month) / 12)))              \
        + ((year + 4800 - ((14 - month) / 12)) / 4)                \
        - ((year + 4800 - ((14 - month) / 12)) / 100)              \
        + ((year + 4800 - ((14 - month) / 12)) / 400)              \
        - 32045                                                    \
      ) % 7;
    return out + 1;	// calculation above gives week days from 0 -> monday
    				// to 6 -> sunday
}

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

/**
 * Sets RTC date and time to values provided by diagnostcs call
 * @param lparam date in binary coded decimal 0xYYYYMMDD
 * @param wparam time in binary coded decimal 0xHHMM
 */
uint8_t routine_5254_start (uint32_t lparam, uint16_t wparam)
{

	routine_5254_rtc_time.TimeFormat = LL_RTC_TIME_FORMAT_AM_OR_24;
	routine_5254_rtc_time.Hours = (wparam & 0xFF00) >> 8;
	routine_5254_rtc_time.Minutes = wparam & 0xFF;
	routine_5254_rtc_time.Seconds = 0;

	routine_5254_rtc_date.Day = lparam & 0xFF;
	routine_5254_rtc_date.Month = (lparam & 0xFF00) >> 8;
	routine_5254_rtc_date.Year = (lparam & 0xFFFF0000) >> 16;

	const int weekday = routine_5254_get_weekday_from_date(&routine_5254_rtc_date);
	routine_5254_rtc_date.WeekDay = weekday;

	// enable access to backup domain
	PWR->CR1 |= PWR_CR1_DBP;

	routine_5254_result |=  (0x00FF & LL_RTC_DATE_Init(RTC, LL_RTC_FORMAT_BIN, &routine_5254_rtc_date));
	routine_5254_result |=  (0xFF00 & LL_RTC_TIME_Init(RTC, LL_RTC_FORMAT_BIN, &routine_5254_rtc_time)) << 8;

	PWR->CR1 &= (0xFFFFFFFFu ^ PWR_CR1_DBP);

	return KISS_ROUTINE_RETVAL_SUCCESSFULLY_STARTED;

}

// this doesn't have stop function, because it is synchronous

/**
 * Returns zero if RTC date and time has been updated successfully, or non zero in case of error
 */
uint16_t routine_5254_get_result (void)
{
    return routine_5254_result;
}
