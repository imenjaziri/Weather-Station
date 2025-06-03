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
#include "message_buffer.h"
#include "cmsis_os.h"
extern uint8_t SF_Lora_Value;
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
void SetAltGPS_f(char* arg);
void GetAltGPS_f(char* arg);
void SetLatGPS_f(char* arg);
void GetLatGPS_f(char* arg);
void SetTimeGPS_f(char* arg);
void GetTimeGPS_f(char* arg);
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
