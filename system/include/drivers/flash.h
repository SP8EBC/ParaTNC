#ifndef __FLASH_H
#define __FLASH_H

#ifndef __FLASH

int KEY1 = 0x45670123;
int KEY2 = 0xCDEF89AB;

#include <stm32f10x.h>

unsigned short int page_data[1024];	// tablica przechowuje zawartosc zmienianej strony pamiêci flash
										// zajmuje ona 2KB w pamiêci operacyjnej czyli tyle ile ma jedna stron


int FlashUnlock(void);
void FlashLock(void);
void ProgramFlashFromAddr(unsigned int addr, int* data);
void ProgramFlashFromInt(unsigned int addr, int data);
void EraseFlashPage(unsigned int addr);



#endif

#ifdef __FLASH

extern int FlashUnlock(void);
extern void FlashLock(void);
extern void ProgramFlashFromAddr(unsigned int addr, int* data);
extern void ProgramFlashFromInt(unsigned int addr, int data);
extern void EraseFlashPage(unsigned int addr);

#endif

#endif

