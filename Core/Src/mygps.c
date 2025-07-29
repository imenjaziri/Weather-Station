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
//#ifdef GPS

#include "rtc.h"
#include "mygps.h"
#include "freertos.h"
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
GPS_RTC gps_to_rtc={0};
GPS_IHM gps_to_ihm={0.0};
// xQueueHandle GpsToIhm;
extern QueueHandle_t GpsToIhm;
volatile uint8_t gps_rtc_updated = 0;
char buffx[300];

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
			gps_to_rtc.hours = MyGps.hours;
			gps_to_rtc.minutes = MyGps.minutes;
			gps_to_rtc.seconds = MyGps.seconds;
			gps_to_rtc.day = MyGps.day;
			gps_to_rtc.month = MyGps.month;
			gps_to_rtc.year = MyGps.year;
		/*	snprintf(buffx, sizeof(buffx), "Parsed GPS Time: %02d:%02d:%02d, Date: %02d-%02d-%02d\r\n",
					MyGps.hours, MyGps.minutes, MyGps.seconds, MyGps.day, MyGps.month, MyGps.year);
			HAL_UART_Transmit(&huart2, (uint8_t*)buffx, strlen(buffx), HAL_MAX_DELAY);*/
			GPS_GetFromRTC(&gps_to_rtc);};
		//	xQueueSend(GpsToIhm, &gps_to_ihm,0);

		osDelay(1000);
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
		/*	HAL_UART_Transmit(&huart2, (const uint8_t *)"GPS Message sent different from buffer data\r\n",strlen("GPS Message sent different from buffer data\r\n"),100);*/
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
void GPS_GetFromRTC(GPS_RTC *gps_rtc){
	RTC_TimeTypeDef rtc_time;
	RTC_DateTypeDef rtc_date;

	HAL_RTC_GetTime(&hrtc, &rtc_time, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &rtc_date, RTC_FORMAT_BIN);

	// Calcul différence temps en minutes
	int rtc_minutes_total = rtc_time.Hours * 60 + rtc_time.Minutes;
	int gps_minutes_total = gps_rtc->hours * 60 + gps_rtc->minutes;
	int diff_minutes = abs(rtc_minutes_total - gps_minutes_total);

	// Vérification date
	int rtc_date_match = (rtc_date.Date == gps_rtc->day) &&
                         (rtc_date.Month == gps_rtc->month) &&
                         (rtc_date.Year == gps_rtc->year);

	// Synchroniser RTC si écart > 5 min ou si la date diffère
	if (diff_minutes >= 5 || !rtc_date_match) {
		RTC_SetFromGPS(gps_rtc);
		gps_rtc_updated = 1;
	/*	HAL_UART_Transmit(&huart2,
            (const uint8_t*)"RTC synchronized from GPS\r\n",
			strlen("RTC synchronized from GPS\r\n"),
			1000);*/
	}
}
void RTC_SetFromGPS(GPS_RTC *gps_rtc)
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
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if(huart->Instance==huart1.Instance)
	{
		GPS_UART_CallBack();
	}
}
//#endif
