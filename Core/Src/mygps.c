/*
 * MyGps.c
 *
 *  Created on: Jun 12, 2025
 *      Author: ThinkPad

 * gps.c
 *
 *  Created on: Jun 4, 2025
 *      Author: ThinkPad
 */
#ifndef gps
#include "rtc.h"
#include "mygps.h"
#define UART_RX_BUFFER_SIZE  500
const size_t xGpsMessageBufferSizeBytes = 500;
uint8_t UART1_RxBuffer[UART_RX_BUFFER_SIZE] = {0};
uint16_t RxDataLen = 0;
MessageBufferHandle_t GpsMessageBufferHandle;
uint8_t RxIdx;
uint8_t NMEA_ToParse[NMEA_BUFFER_SIZE]={0};
char received_nmea[NMEA_BUFFER_SIZE]={0};
BaseType_t xHigherPriorityTaskWokenGps = pdFALSE;
uint8_t xGpsBytesSent;
uint8_t gps_message_buffer_flag;
uint8_t GPS_Valid=1;
uint8_t Assign_Values=0;
void Start_GPS_Task(void const * argument)
{
	GpsMessageBufferHandle = xMessageBufferCreate(xGpsMessageBufferSizeBytes);
	if( GpsMessageBufferHandle != NULL )
	{
	}
	else
	{
		HAL_UART_Transmit(&huart2,(uint8_t*)"Error in MessageBuffer Creation\r\n", 34, 100);
	}
	GPS_Init();
	for(;;)
	{
		xMessageBufferReceive( GpsMessageBufferHandle, received_nmea, sizeof(received_nmea), portMAX_DELAY);
		Sentence_parse((char*)received_nmea);
		if (Assign_Values==1){
			GPS_Nmea_time();
			GPS_Nmea_Date();
			GPS_GetFromRTC(&MyGps);};
		osDelay(100);
	}
}
void GPS_UART_CallBack() {
	if (rx_data != '\n' && rx_index < sizeof(rx_buffer)) {
		rx_buffer[rx_index++] = rx_data;
	} else {
		rx_buffer[rx_index] = '\0';
		memcpy(NMEA_ToParse,rx_buffer,rx_index);
		xGpsBytesSent=xMessageBufferSendFromISR(GpsMessageBufferHandle,NMEA_ToParse,strlen((char*)NMEA_ToParse),&xHigherPriorityTaskWokenGps);
		if( xGpsBytesSent != strlen((char*)NMEA_ToParse))
		{
			HAL_UART_Transmit(&huart2, (const uint8_t *)"GPS Message sent different from buffer data\r\n",strlen("GPS Message sent different from buffer data\r\n"),100);
		}
		memset(rx_buffer,0,sizeof(rx_buffer));
		rx_index = 0;

	}
	HAL_UART_Receive_IT(GPS_USART, &rx_data, 1);
}

void Sentence_parse(char* str){
	if (GPS_validate(str)) {
		GPS_parse(str);
		Assign_Values=1;
	}
	else GPS_Valid=0;
}
void GPS_GetFromRTC(GPS_Data *gps_rtc){
	RTC_TimeTypeDef rtc_time;
	RTC_DateTypeDef rtc_date;
	HAL_RTC_GetTime(&hrtc, &rtc_time, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &rtc_date, RTC_FORMAT_BIN);
	int rtc_minutes_total = rtc_time.Hours * 60 + rtc_time.Minutes;
	int gps_minutes_total = gps_rtc->hours * 60 + gps_rtc->minutes;
	int diff_minutes = abs(rtc_minutes_total - gps_minutes_total);
	if (diff_minutes >= 5)
		RTC_SetFromGPS(gps_rtc);
	/*	HAL_UART_Transmit(&huart2, (const uint8_t*)"RTC synchronized from GPS\r\n", strlen("RTC synchronized from GPS\r\n"), 1000);
	} else {
		HAL_UART_Transmit(&huart2, (const uint8_t*)"RTC sync skipped (diff < 5 min)\r\n", strlen("RTC sync skipped (diff < 5 min)\r\n"), 1000);
	}*/
}
void RTC_SetFromGPS(GPS_Data *gps_rtc)
{

	RTC_TimeTypeDef sTime={0};
	RTC_DateTypeDef sDate ={0} ;
	sTime.Hours = gps_rtc->hours;
	sTime.Minutes = gps_rtc->minutes;
	sTime.Seconds = gps_rtc->seconds;
	sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	sTime.StoreOperation = RTC_STOREOPERATION_RESET;
	sDate.Year = gps_rtc->year;
	sDate.Month = gps_rtc->month;
	sDate.Date = gps_rtc->day;
	sDate.WeekDay = 0;


	if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
	{
		Error_Handler();
	}
	if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
	{
		Error_Handler();
	}
}
#endif
