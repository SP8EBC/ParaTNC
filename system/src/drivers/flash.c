#include "drivers/flash.h"

int FlashUnlock(void) {
	FLASH->KEYR = KEY1;
	FLASH->KEYR = KEY2;
	if ((FLASH->CR & FLASH_CR_LOCK) == FLASH_CR_LOCK)
		return -1;
	return 0;
}

void FlashLock(void) {
	FLASH->CR |= FLASH_CR_LOCK;
}

void ProgramFlashFromAddr(unsigned int addr, int* data){
	// pod ka�dym adresem jest zapami�tane 8 bit�w czyli jeden bajt!!

	unsigned short int dat_m, dat_l;			// dane rozdzielone na dwie cz�ci
	unsigned short int rw_addr = 0;				// adres aktualnie zapisywanej lub odczytywanej strony
	int fi;
	int pagenum;								// zmienna przechowuje numer strony w kt�rej b�d� zapisywane dane
	int in_page_off;							// ilo�� 16bitowych przesuni�� od pocz�tku strony do kt�rych maj� by� zapisane dane 
	pagenum = (int)((addr - 0x8000000)/2048);	// obliczanie numeru strony do zaprogramowania
	in_page_off = (addr - (0x8000000 + pagenum * 2048)) / 2;	// obliczanie przesuni�cie 16bitowe od pocz�tku strony
	dat_l = (unsigned short)(*data & 0xFFFF);					// 16 najmniej znacz�cych
	dat_m = (unsigned short)((*data & (0xFFFF << 16)) >> 16);	// 16 najbardziej znacz�cych bit�w
	for (fi = 0; fi < 1024; fi++) {
		page_data[fi] = *(unsigned short int*)(rw_addr + 0x8000000 + pagenum * 2048);
		// przepisywanie zawarto�ci zmienianej strony do pami�ci RAM. Przepisywanie odbywa si� po 16 bit�w
		rw_addr += 2;
	}
	page_data[in_page_off] = dat_l;
	page_data[in_page_off+1] = dat_m;  
	FlashUnlock();
	EraseFlashPage(addr);
	FLASH->CR |= FLASH_CR_PG;			// w��czenie zapisu
	rw_addr = 0;
	for (fi = 0; fi < 1024; fi++) {
		*(unsigned short int*)(rw_addr + 0x8000000 + pagenum * 2048) = page_data[fi];
		while ((FLASH->SR & FLASH_SR_BSY) == FLASH_SR_BSY);
		rw_addr += 2;
	}
	FLASH->CR &= (0xFFFFFFFF ^ FLASH_CR_PG);		// wy��czenie zapisu
	FlashLock();
}


void ProgramFlashFromInt(unsigned int addr, int data){
	
//	unsigned short int dat_m;	// dane rozdzielone na dwie cz�ci
//	unsigned short int dat_l;
//	FlashUnlock();
//	EraseFlashPage(addr);
//	dat_l = (unsigned short)data & 0xFFFF;		// 16 najmniej znacz�cych
//	dat_m = (data & (0xFFFF << 16)) >> 16;		// 16 najbardziej znacz�cych bit�w
//	FLASH->CR |= FLASH_CR_PG;			// w��czenie zapisu
//	if ((FLASH->CR & FLASH_CR_LOCK) == 0 && (FLASH->SR & FLASH_SR_BSY) == 0) {
//		*(unsigned short int*) addr = dat_l;					// zapis najbardziej znacz�cych bit�w
//		while ((FLASH->SR & FLASH_SR_BSY) == FLASH_SR_BSY);
//	}
//	if ((FLASH->CR & FLASH_CR_LOCK) == 0 && (FLASH->SR & FLASH_SR_BSY) == 0) {
//		addr += 2;
//		*(unsigned short int*) addr = dat_m;					// zapis najnniej znacz�cych bit�w
//		while ((FLASH->SR & FLASH_SR_BSY) == FLASH_SR_BSY);
//	}
//	FLASH->CR &= (0xFFFFFFFF ^ FLASH_CR_PG);		// wy��czenie zapisu
//	FlashLock();
	unsigned short int dat_m, dat_l;	// dane rozdzielone na dwie cz�ci
	unsigned short int rw_addr = 0;			// adres zapisywanej lub odczytywanej strony
	int fi;
	int pagenum;							// zmienna przechowuje numer strony w kt�rej b�d� zapisywane dane
	int in_page_off;						// ilo�� 16bitowych przesuni�� od pocz�tku strony do kt�rych maj� by� zapisane dane 
	pagenum = (int)((addr - 0x8000000)/2048);	// obliczanie numeru strony do zaprogramowania
	in_page_off = (addr - (0x8000000 + pagenum * 2048)) / 2;	// obliczanie przesuni�cie 16bitowe od pocz�tku strony
	dat_l = (unsigned short)data & 0xFFFF;    	// 16 najmniej znacz�cych
	dat_m = (data & (0xFFFF << 16)) >> 16;	  	// 16 najbardziej znacz�cych bit�w
	for (fi = 0; fi < 1024; fi++) {
		page_data[fi] = *(unsigned short int*)(rw_addr + 0x8000000 + pagenum * 2048);
		// przepisywanie zawarto�ci zmienianej strony do pami�ci RAM. Przepisywanie odbywa si� po 16 bit�w
		rw_addr += 2;
	}
	page_data[in_page_off] = dat_l;
	page_data[in_page_off+1] = dat_m;  
	FlashUnlock();
	EraseFlashPage(addr);
	FLASH->CR |= FLASH_CR_PG;			// w��czenie zapisu
	rw_addr = 0;
	for (fi = 0; fi < 1024; fi++) {
		*(unsigned short int*)(rw_addr + 0x8000000 + pagenum * 2048) = page_data[fi];
		while ((FLASH->SR & FLASH_SR_BSY) == FLASH_SR_BSY);
		rw_addr += 2;
	}
	FLASH->CR &= (0xFFFFFFFF ^ FLASH_CR_PG);		// wy��czenie zapisu
	FlashLock();

}

void EraseFlashPage(unsigned int addr) {
	if((FLASH->SR & FLASH_SR_BSY) == 0) {
		FLASH->CR |= FLASH_CR_PER;
		FLASH->AR = addr;
		FLASH->CR |= FLASH_CR_STRT;
		while ((FLASH->SR & FLASH_SR_BSY) == FLASH_SR_BSY);
		FLASH->CR &= (0xFFFFFFFF ^ FLASH_CR_PER); 
			
	}
}
