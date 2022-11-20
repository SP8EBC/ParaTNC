#ifndef __AFSK_PR_H
#define __AFSK_PR_H



#ifdef __cplusplus
extern "C"
{
#endif

void ADCStartConfig(void);
void DACStartConfig(void);

#ifdef STM32L471xx

void ADCStop(void);
void DACStop(void);

#endif

#ifdef __cplusplus
}
#endif



#endif
