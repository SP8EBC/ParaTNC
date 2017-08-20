#ifndef __USER_INTERF_H
#define __USER_INTERF_H

#ifndef __USER_INTERF

#include <stdlib.h>
#include <string.h>
#include <stm32f10x.h>
#include "locale.h"

int ascii[] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39 };
int key;

int FlightStatsC = 8;
int MainMenuC = 2;


// definicje napis�w wy�wietlacza
char* AirTimeLine = "AIRTIME:      s";
char* Alti1Line = "ALTI1:        m";
char* Alti2Line = "ALTI2:        m"; 
char* VarioLine = "VARIO:      m/s";
char* MaxClimbingLine = "MAX CLMB:       ";
char* MaxDescendingLine = "MAX DSCND:      ";
char* VarioAvgLine = "VARIO AVG:      ";
char* VarioGraphLine = "                ";
char* MaxAltGain = "MxAltS:       m";
char* NullLineS = "     ";

char* testing1 = "TEMP:     oC";   // 11
char* testing2 = "BARO:       hPa";  // 13
char* testing3 = "ALTI:       m";	 // 13
char* testing4 = "VARIO:      m/s";  // 13


// definicje tablic char�w kt�re zawieraj� wzorki u�o�one z kratek do wy�wietlania
// jako bargraf wariometru
char* Bargraph1h = "#       ";
char* Bargraph2h = "##      ";
char* Bargraph3h = "###     ";
char* Bargraph4h = "####    ";
char* Bargraph5h = "#####   ";
char* Bargraph6h = "######  ";
char* Bargraph7h = "####### ";
char* Bargraph8h = "########";
char* Bargraph9h = " #######";
char* Bargraph10h= "  ######";
char* Bargraph11h= "   #####";
char* Bargraph12h= "    ####";
char* Bargraph13h= "     ###";
char* Bargraph14h= "      ##";
char* Bargraph15h= "       #";
char* Bargraph16h= "        ";

char* Bargraphc= "-";

int j = 0;					// do cel�w testowych

struct FlightData {
	// definicja struktury u�ywanej do zapisu parametr�w lotu do pami�ci flash
	float MaxSink;
	float MaxClimbing;
	float MaxAltiGain;
	float BestThermalHeigh;
	int FlightTime;
	float StartAltitude;
	float LandingAltitude;
};


void Delay1ms(void);
void Delay10ms(void);
void FixString(char* input, int len);

#define __SERIAL
#include "drivers/serial.h"
//extern void SrlSendData(char* data);
//extern int SrlTXing;

//#define __FLASH
//#include "drivers/flash.h"

//void FlightStats(struct FlightData lot);

char sss[10];

#endif

#ifdef __USER_INTERF
 
extern int GetCodeFromDigit(int input);
extern void int2string(int input, char* output);
extern void float2string(float input, char* output, int accur);
extern void MainScreen(float vario, float alti, int fltime, int settings);
extern void TableCpy(int* copy_from, int* copy_to);
extern void Delay1ms(void);
extern void Delay10ms(void);
extern void FixString(char* input, int len);
extern void FlightStats(struct FlightData lot);
extern void MainMenu(void);

#endif

#endif
