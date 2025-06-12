/*
 * MyGps.c
 *
 *  Created on: Jun 12, 2025
 *      Author: ThinkPad
 */

/*
 * gps.c
 *
 *  Created on: Jun 4, 2025
 *      Author: ThinkPad
 */
#include "mygps.h"
#define UART_RX_BUFFER_SIZE  100
#define NMEA_BUFFER_SIZE  100
const size_t xGpsMessageBufferSizeBytes = 100;
uint8_t UART1_RxBuffer[UART_RX_BUFFER_SIZE] = {0};
uint16_t RxDataLen = 0;
MessageBufferHandle_t GpsMessageBufferHandle;
uint8_t RxIdx;
uint8_t NMEA_ToParse[NMEA_BUFFER_SIZE]={0};
char received_nmea[NMEA_BUFFER_SIZE]={0};
BaseType_t xHigherPriorityTaskWokenGps = pdFALSE;
uint8_t xGpsBytesSent;


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

	HAL_UARTEx_ReceiveToIdle_DMA(&huart1, UART1_RxBuffer, UART_RX_BUFFER_SIZE);

	for(;;)
	{
		xMessageBufferReceive( GpsMessageBufferHandle, received_nmea, sizeof(received_nmea), portMAX_DELAY);
		Sentence_parse((char*)received_nmea);
		memset(received_nmea,0,sizeof(received_nmea));

	}
}
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	if (huart->Instance == USART1) {
		RxDataLen = Size;
		RxIdx=RxDataLen;
		UART1_RxBuffer[Size] = '\0';
		memcpy(NMEA_ToParse,UART1_RxBuffer,RxIdx);
		xGpsBytesSent=xMessageBufferSendFromISR(GpsMessageBufferHandle,NMEA_ToParse,strlen((char*)NMEA_ToParse),&xHigherPriorityTaskWokenGps);
		if( xGpsBytesSent != strlen((char*)NMEA_ToParse))
		{
			HAL_UART_Transmit(&huart2, (const uint8_t *)"GPS Message sent different from buffer data\r\n",strlen("GPS Message sent different from buffer data\r\n"),100);
		}
		memset(UART1_RxBuffer,0,UART_RX_BUFFER_SIZE);
		RxIdx = 0;

		HAL_UARTEx_ReceiveToIdle_DMA(&huart1, UART1_RxBuffer, UART_RX_BUFFER_SIZE);
	}
}
void Sentence_parse(char *sentence)
{
	sentence= strtok((char*)received_nmea, "\r\n");
	while (sentence != NULL) {
	        if (GPS_validate(sentence)) {
	            GPS_parse(sentence);
	        }
	        sentence = strtok(NULL, "\r\n");
	    }

}
