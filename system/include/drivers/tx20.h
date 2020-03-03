#ifndef __TX20_H
#define __TX20_H

#include "station_config.h"
#include <stdint.h>

#ifdef _ANEMOMETER_TX20

#define VNAME TX20	// nazwa tworzonej zmiennej strukturalnej

typedef struct {
	float WindSpeed;
	short WindDirX;
	short WindDirY;
	int Checksum;
	int CalcChecksum;
} DecodedData;

#define TX20_BUFF_LN 20

typedef struct Anemometer {
	char BitSampler;
	/* Zmienna przechowuj�ca stan automatu sampluj�cego bity.
	Jest konieczna poniewa� sa one samplowane 2x czesciej niz wynosi faktyczna
	predkosc transmisji tak aby kazdy na pewno zostal poprawnie odczytany
	*/
	unsigned long long int BitQueue;
	/* Kolejka dla samplowanych bitow */
	unsigned short int QueueLenght;
	/* D�gosc kolejki rozumiana jako ilosc zapisanych bitow */
	char FrameRX;
	/* Sygnalizacja detekcji poczatku ramki */
	unsigned short int FrameBitCounter;
	/* Licznik bitow odebranych z ramki */
	char ReceiveDone;
	/* Zmienna sygnalizujaca zakonczenie odbierania ramki */
	#define START_FRAME 0x1B
	/* Definicje pinow do ktorych podlaczony jest anemomter	*/
	#define PORT GPIOB
	#define DTR	8
	#define TX	9
	/* Z powodu pewnych u�omno�ci preprocesora w toolchainie RealView opr�cz podawania
	   nazwy portu nale�y wpisa� jego numer. 0 odpowiada GPIOA, 1 GPIOB itd. */
	#define PORTNUM 1
	#define TIMER TIM1
	/* Podobnie jak w przypadku port�w GPIO, podobny problem dotyczy timera */ 
	#define TIMNUMBER 1
	/* Aktualne odczyty zwracane przez czujnik */
	DecodedData Data;
	/* Licznik do poruszania si� po tablicy HistoryAVG*/
	unsigned char MeasCounter;
	/* Indeks poprzednio zapisanych pomiarow*/
	unsigned char PrevMeasCounter;
	/* Historia odczytow i usredniona wartosc z ostatnich 15 pomiarow  */	
//	DecodedData HistoryAVG[TX20_BUFF_LN];
	
	unsigned char OddEven;
} Anemometer;

extern Anemometer VNAME;

#ifdef __cplusplus
extern "C"
{
#endif

void tx20_batch(void);
void tx20_init(void);
float tx20_data_average(void);
void tx20_data_parse(void);
uint16_t tx20_get_scaled_windspeed(void);
uint16_t tx20_get_wind_direction(void);

#ifdef __cplusplus
}
#endif


extern Anemometer VNAME;

#endif // #ifdef _ANEMOMETER_TX20

#endif // #ifndef __TX20_H
