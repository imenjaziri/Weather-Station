/*
 * sht40_driver.h
 *
 *  Created on: Jul 13, 2025
 *      Author: ThinkPad
 */

#ifndef INC_SHT40_DRIVER_H_
#define INC_SHT40_DRIVER_H_
#include "stm32l4xx_hal.h"
void sht40_init(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef sht40_read_temperature_humidity(float *temperature, float *humidity);

#endif /* INC_SHT40_DRIVER_H_ */
/*	float adc_val = 0.0;

	uint8_t txt[] = "Message from sensors task :\r\n";
	char value_buff[120] = "";

	// Envoi du message initial
	HAL_UART_Transmit(&huart2, txt, strlen((char*)txt), 1000);

	// Lecture ADC
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 1000);
	adc_val = (float)HAL_ADC_GetValue(&hadc1);

	HAL_ADC_Stop(&hadc1);
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 1000);
	adc_val = (float)HAL_ADC_GetValue(&hadc1);
	float voltage = (adc_val / 4095.0f) * 3.3;
	float lux=(voltage/ 3.3f) * 6000.0f;
	float rs = lux * 0.00072f;
	HAL_ADC_Stop(&hadc1);

	float voltage = (adc_val / 4095.0f) * 3.3f;
	float humidity_percent = (1.0f - voltage / 3.3f) * 100.0f;
	sprintf(value_buff, "Voltage = %.2f V, humidity: %f\r\n",  voltage,humidity_percent);
	HAL_UART_Transmit(&huart2, (uint8_t*)value_buff, strlen(value_buff), 1000);
	sprintf(value_buff, "ADC = %.0f, Voltage = %.2f V, Lux : %f, Rs : %f\r\n", adc_val, voltage,lux,rs);
	HAL_UART_Transmit(&huart2, (uint8_t*)value_buff, strlen(value_buff), 1000);
*/
