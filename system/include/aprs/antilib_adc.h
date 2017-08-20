//=======================================================================================
// STM32 AntiLib Project
// Module : ADC
// Description : Macros for configure ADC
// Author : Radoslaw Kwiecien
// e-mail : radek(at)dxp.pl
// website : http://en.radzio.dxp.pl
//=======================================================================================
#ifndef ANTILIB_ADC_H
#define ANTILIB_ADC_H

#define SAMPLE_TIME_1_5		0
#define SAMPLE_TIME_7_5		1
#define SAMPLE_TIME_13_5	2
#define SAMPLE_TIME_28_5	3
#define SAMPLE_TIME_41_5	4
#define SAMPLE_TIME_55_5	5
#define SAMPLE_TIME_71_5	6
#define SAMPLE_TIME_239_5	7


#define ADC_SAMPLE_TIME0(x)			(x << 0)
#define ADC_SAMPLE_TIME1(x)			(x << 3)
#define ADC_SAMPLE_TIME2(x)			(x << 6)
#define ADC_SAMPLE_TIME3(x)			(x << 9)
#define ADC_SAMPLE_TIME4(x)			(x << 12)
#define ADC_SAMPLE_TIME5(x)			(x << 15)
#define ADC_SAMPLE_TIME6(x)			(x << 18)
#define ADC_SAMPLE_TIME7(x)			(x << 21)
#define ADC_SAMPLE_TIME8(x)			(x << 24)
#define ADC_SAMPLE_TIME9(x)			(x << 27)

#define ADC_SAMPLE_TIME10(x)		(x << 0)
#define ADC_SAMPLE_TIME11(x)		(x << 3)
#define ADC_SAMPLE_TIME12(x)		(x << 6)
#define ADC_SAMPLE_TIME13(x)		(x << 9)
#define ADC_SAMPLE_TIME14(x)		(x << 12)
#define ADC_SAMPLE_TIME15(x)		(x << 15)
#define ADC_SAMPLE_TIME16(x)		(x << 18)
#define ADC_SAMPLE_TIME17(x)		(x << 21)


#define ADC_SEQUENCE_LENGTH(x)	(x << 20)

// SQR3
#define ADC_SEQ1(x)		(x << 0)
#define ADC_SEQ2(x)		(x << 5)
#define ADC_SEQ3(x)		(x << 10)
#define ADC_SEQ4(x)		(x << 15)
#define ADC_SEQ5(x)		(x << 20)
#define ADC_SEQ6(x) 	(x << 25)
// SQR2
#define ADC_SEQ7(x)		(x << 0)
#define ADC_SEQ8(x)		(x << 5)
#define ADC_SEQ9(x)		(x << 10)
#define ADC_SEQ10(x) 	(x << 15)
#define ADC_SEQ11(x)	(x << 20)
#define ADC_SEQ12(x) 	(x << 25)
// SQR1
#define ADC_SEQ13(x) 	(x << 0)
#define ADC_SEQ14(x) 	(x << 5)
#define ADC_SEQ15(x) 	(x << 10)
#define ADC_SEQ16(x)	(x << 15)

#endif
