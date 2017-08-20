#include "drivers/user_interf.h"


//void FixString(char* input, int len) {
//	// funkcja ma usuwa� zb�dne znaczniki ko�ca ci�gu powsta�e po funkcji strcpy
//	// zamienia wszystkie naporkane przed ko�cem znaki /0 na spacj�
//	int i = 1;
//	for(i; (i<32 && i<len); i++) {
//		if(*(input+i) == '\0' && i != (len+1)  )
//			*(input+i) = 0x20;
//	*(input+len) = '\0';
//	}
//}


void Delay1ms(void) {
	int i;
	for (i = 0; i<0x3E5; i++);
}

void Delay10ms(void) {
	int i;
	for (i = 0; i<0x3E50; i++);
}











