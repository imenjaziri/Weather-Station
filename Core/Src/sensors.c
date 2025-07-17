/*
 * sensors.c
 *
 *  Created on: Jul 10, 2025
 *      Author: ThinkPad
 */
#include "sensors.h"
volatile uint32_t last_press_time = 0;
uint8_t counter = 0;
uint8_t precipitation_flag=0;
extern osThreadId Sensors_TaskHandle;
#define SIGNAL_RAIN_DETECTED  (1 << 0)

void Start_Sensors_Task(void const * argument) {
	/*for (uint8_t addr = 1; addr < 127; addr++) {
		if (HAL_I2C_IsDeviceReady(&hi2c2, addr << 1, 2, 10) == HAL_OK) {
			char msg[64];
			snprintf(msg, sizeof(msg), "I2C device found at 0x%02X\r\n", addr);
			HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
		}
	}
	SensorReadings_t myData;
			if (GetStableADCReading_Polling(&myData)) {
				float Vsoil = myData.soil_voltage;
				float Vlight = myData.light_voltage;

				float humidity_percent = (1.0f - Vsoil / 3.3f) * 100.0f;
				float lux = Vlight * 200.0f;   // Vlight en V -> lux
				float rs = (3.3f * 10000.0f / Vlight) - 10000.0f;

				char msg[128];
				snprintf(msg, sizeof(msg),
						"✅ Humidité: %.1f%% | Lux: %.0f lx | Rs: %.0fΩ\r\n",
						humidity_percent, lux, rs);
				HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
			}		float T = 0, RH = 0;
			if (sht40_read_temperature_humidity(&T, &RH) == HAL_OK) {
				char buf[64];
				snprintf(buf, sizeof(buf), "Temp: %.2f °C, RH: %.2f %%\r\n", T, RH);
				HAL_UART_Transmit(&huart2, (uint8_t*)buf, strlen(buf), portMAX_DELAY);
			} else {
				HAL_UART_Transmit(&huart2, (uint8_t*)"Erreur capteur SHT40\r\n", 23, HAL_MAX_DELAY);
			}*/
	for (;;) {
		osEvent evt = osSignalWait(SIGNAL_RAIN_DETECTED, osWaitForever);
		        if (evt.status == osEventSignal) {
		            HAL_UART_Transmit(&huart2, (uint8_t*)"🌧️ Précipitation détectée!\r\n", 30, HAL_MAX_DELAY);
		        }
		        /*if (precipitation_flag) {
		    HAL_UART_Transmit(&huart2, (uint8_t*)" Précipitation détectée!\r\n", strlen(" Précipitation détectée!\r\n"), 1000);
		    precipitation_flag = 0;
		}*/
		osDelay(2000);
	}
}


int GetStableADCReading_Polling(SensorReadings_t *result) {
	float last_soil = -1, last_light = -1;
	float current_soil, current_light;

	for (int i = 0; i < MAX_ATTEMPTS; ++i) {
		HAL_ADC_Start(&hadc1);
		HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
		uint32_t raw_light = HAL_ADC_GetValue(&hadc1);
		HAL_ADC_Stop(&hadc1);

		HAL_ADC_Start(&hadc2);
		HAL_ADC_PollForConversion(&hadc2, HAL_MAX_DELAY);
		uint32_t raw_soil = HAL_ADC_GetValue(&hadc2);
		HAL_ADC_Stop(&hadc2);

		current_soil = (raw_soil / ADC_RES) * ADC_VREF;
		current_light = (raw_light / ADC_RES) * ADC_VREF;

		if (i > 0) {
			float delta_soil = fabs(current_soil - last_soil);
			float delta_light = fabs(current_light - last_light);

			if (delta_soil < STABILITY_THRESHOLD && delta_light < STABILITY_THRESHOLD) {
				result->soil_voltage = current_soil;
				result->light_voltage = current_light;
				return 1;
			}
		}

		last_soil = current_soil;
		last_light = current_light;

		osDelay(200);
	}

	return 0;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin==GPIO_PIN_1)
	{
		uint32_t now = HAL_GetTick();
		if (now - last_press_time > 200) {
			last_press_time = now;
			counter++;
		}
		if (counter >= 2) {
			precipitation_flag = 1;
			counter = 0;
		}
        osSignalSet(Sensors_TaskHandle, SIGNAL_RAIN_DETECTED);

	}

}
