/*
 * ihm.h
 *
 *  Created on: May 5, 2025
 *      Author: ThinkPad
 */

#ifndef INC_IHM_H_
#define INC_IHM_H_

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "main.h"
#include "usart.h"
#include "stm32l4xx_hal_uart.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "message_buffer.h"
#include "cmsis_os.h"
#include "rtc.h"
typedef void (*cmdHandler)(char *arg); // parameters: token[2], token[3] (TYPE OF VALUE+VAL), number of arguments (tokens)
typedef struct {
	char* Name;
	char* helper;
	cmdHandler handler;
	uint8_t MenuIndex;
}CMD;
typedef enum {
	Main_Menu,
	Lora_Menu,
	GPS_Menu,
	Sensors_Menu,
	Prediction_Menu,
	SysConfig_Menu
}Menu;
typedef struct {
	uint8_t sf_l;
	uint8_t cr_l ;
	uint8_t bw_l ;
}Lora;
extern Lora LoraValues ;
 typedef struct {
	float alt_gps;
	float lat_gps ;
	float long_gps;
	int hours;
	int minutes;
	int seconds;
	int day;
	int month;
	int year;
	int offset;
}GPS_Data;
extern GPS_Data MyGps;
typedef struct {
	float AirTemp_s;
	float SoilTemp_s ;
	float RelativeHumidity_s ;
	float SoilHumidity_s;
	float AirPressure_s;
	float WindSpeed_s;
	float Kc;
	float Kp;
	float ET0;
	float Radiation_s;
	float ETc;
	float ETcAdj;
}SENSORS;
extern SENSORS SensorsValues;
void MainMenu(void);
void LoraMenu(char* arg);
void SensorsMenu(char* arg);
void GPSMenu(char* arg);
void SysConfigMenu(char* arg);
void ParseCommand();
void SetSF_f(char* arg);
void GetSF_f(char* arg);
void SaveSF_f(char* arg);
void SetCR_f(char* arg);
void GetCR_f(char* arg);
void SaveCR_f(char* arg);
void SetBW_f(char* arg);
void GetBW_f(char* arg);
//GPS Functions
void SetGPS_f(char* arg);
void GetGPS_f(char* arg);
void SaveGPS_f(char* arg);
void GetAltGPS_f(char* arg);
void GetLatGPS_f(char* arg);
void GetLongGPS_f(char* arg);
void GetTimeGPS_f(char* arg);
void SetLocalTimeOffset_f(char* arg);
void GetLocalTimeOffset_f(char* arg);
//Prediction Functions
//void GetNextPass_f(char* arg);
//Sensors Functions
void GetSoilTemp_f(char* arg);
void GetAirTemp_f(char* arg);
void GetRelativeHumidity_f(char* arg);
void GetSoilHumidity_f(char* arg);
void GetWindSpeed_f(char* arg);
void SetRadiation_f(char* arg);
void GetRadiation_f(char* arg);
void SetKc_f(char* arg);
void GetKc_f(char* arg);
void SetKp_f(char* arg);
void GetKp_f(char* arg);
void GetET0_f(char* arg);
void SetET0_f(char* arg);
void SetETC_f(char* arg);
void GetETC_f(char* arg);
void SetETCadj_f(char* arg);
void GetETCadj_f(char* arg);
void SetAirPressure_f(char* arg);
void GetAirPressure_f(char* arg);
void SetHeigh_f(char* arg);
void GetHeigh_f(char* arg);
//SysConf functions
void Save_f(char* arg);
void Restore_f(char* arg);
void tokenization(char *str);


#endif /* INC_IHM_H_ */
