#ifndef MAIN_H_
#define MAIN_H_

#include "aprs/ax25.h"

extern uint32_t master_time;

extern AX25Ctx ax25;
extern Afsk a;

extern AX25Call path[3];
extern uint8_t path_len;
extern uint8_t aprs_msg_len;
extern char aprs_msg[128];

extern char after_tx_lock;

extern unsigned char BcnInterval, WXInterval, BcnI, WXI, TelemInterval, TelemI;
extern unsigned short rx10m, tx10m, digi10m, kiss10m;
extern int t;

extern float temperature;
extern float td;
extern double pressure;

#endif
