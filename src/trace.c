/*
 * trace.c
 *
 *  Created on: Jan 9, 2026
 *      Author: mateusz
 */

#include <stm32l4xx.h>
#include <stdbool.h>

void Watchpoint_00001917_At_2000B438_Init(void)
{
    /* Make sure DWT is accessible */
    if ((DWT->CTRL & DWT_CTRL_NUMCOMP_Msk) == 0) {
        /* No comparators implemented – give up or assert here */
        return;
    }

    /* Optional: clear previous comparator configuration */
    DWT->FUNCTION0 = 0;
    DWT->FUNCTION1 = 0;

    /* -------- Comparator 0: address 0x2000B438 -------- */
    DWT->COMP0 = 0x2000B438UL;
    DWT->MASK0 = 0;         /* no masking → watch exactly one word (aligned) */
    /* Leave FUNCTION0 = 0: it is used only as an address filter for comp1 */

    /* -------- Comparator 1: data value 0x00001917 (32‑bit word) -------- */
#undef COMP1

    /* For 32‑bit word writes, store the full 32‑bit value in COMP1. */
    DWT->COMP1 = 0x00001937UL;

    DWT->MASK1 = 0;   /* no masking of the value itself */

    /* FUNCTION1 fields (see CMSIS core_cm4.h / ARM docs)[30]:
       - FUNCTION[3:0] = 0x6 → generate debug event on *write* data access
       - DATAVMATCH    = 1   → enable data value comparison
       - DATAVSIZE     = 10b → word (4 bytes)
       - DATAVADDR0/1  = 0   → use comparator 0 as address filter (default when 0)
    */
    DWT->FUNCTION1 =
          DWT_FUNCTION_DATAVMATCH_Msk          /* enable data value match    */
        | (2U << DWT_FUNCTION_DATAVSIZE_Pos)   /* size = 4 bytes (word)      */
        | 0x7U;                                /* write-data watchpoint       */
}

void Watchpoint_1917_At_2000B438_Init(void)
{
    /* Make sure DWT is accessible */
    if ((DWT->CTRL & DWT_CTRL_NUMCOMP_Msk) == 0) {
        /* No comparators implemented – give up or assert here */
        return;
    }

    /* Optional: clear previous comparator configuration */
    DWT->FUNCTION0 = 0;
    DWT->FUNCTION1 = 0;

    /* -------- Comparator 0: address 0x2000B438 -------- */
    DWT->COMP0 = 0x2000B438UL;
    DWT->MASK0 = 0;         /* no masking → watch exactly one word (aligned) */
    /* Leave FUNCTION0 = 0: it's used only as an address filter for comp1 */

    /* -------- Comparator 1: data value 0x1917 (halfword) -------- */

    /* For data-value watchpoints the DWT compares a 32‑bit COMPx value.
       For halfword size the 16‑bit pattern must be duplicated into both halfwords.[4] */
    uint32_t val16 = 0x1937U;
    DWT->COMP1 = val16 | (val16 << 16);  /* 0x19171917 */

    DWT->MASK1 = 0;   /* no masking of the value itself */

    /* FUNCTION1 fields (see CMSIS core_cm4.h / ARM docs)[30]:
       - FUNCTION[3:0] = 0x6 → generate debug event on *write* data access
       - DATAVMATCH    = 1   → enable data value comparison
       - DATAVSIZE     = 01b → halfword (2 bytes)
       - DATAVADDR0/1  = 0   → use comparator 0 as address filter (default when 0)
    */
    DWT->FUNCTION1 =
          DWT_FUNCTION_DATAVMATCH_Msk          /* enable data value match    */
        | (1U << DWT_FUNCTION_DATAVSIZE_Pos)   /* size = 2 bytes (halfword)  */
        | 0x7U;                                /* write-data watchpoint       */
}

bool debug_monitor_enable(void) {
//  volatile uint32_t *dhcsr = (uint32_t*)0xE000EDF0;
//  if ((*dhcsr & 0x1) != 0) {
//    return false;
//  }
//
//  volatile uint32_t *demcr = (uint32_t*)0xE000EDFC;
//  const uint32_t mon_en_bit = 16;
//  *demcr |= 1 << mon_en_bit;
//
//  // Priority for DebugMonitor Exception is bits[7:0].
//  // We will use the lowest priority so other ISRs can
//  // fire while in the DebugMonitor Interrupt
//  volatile uint32_t *shpr3 = (uint32_t *)0xE000ED20;
//  *shpr3 = 0xff;

    /* Enable trace/DWT and DebugMonitor exceptions */
//    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk   /* enable DWT/ITM/etc */ |
//                        CoreDebug_DEMCR_MON_EN_Msk;  /* route debug events to DebugMon */

    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;   /* enable DWT/ITM/etc */

    /* Set DebugMonitor priority and enable it */
    NVIC_SetPriority(DebugMonitor_IRQn, 0);          /* choose a suitable priority */
    NVIC_EnableIRQ(DebugMonitor_IRQn);

    Watchpoint_00001917_At_2000B438_Init();

  return true;
}

