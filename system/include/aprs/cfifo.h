#ifndef FIFO_H_
#define FIFO_H_

#include <stdint.h>

typedef struct FIFOBuffer
{

	uint8_t * volatile head;
	uint8_t * volatile tail;
	uint8_t * begin;
	uint8_t * end;

} FIFOBuffer;




#endif /* FIFO_H_ */
