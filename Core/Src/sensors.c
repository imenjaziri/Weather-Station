/*
 * sensors.c
 *
 *  Created on: Jul 10, 2025
 *      Author: ThinkPad
 */
#include "sensors.h"
extern osTimerId FanTimerHandle;
extern osThreadId Sensors_TaskHandle;
volatile uint32_t last_press_time = 0;
extern volatile uint8_t debounce_active ;
extern volatile uint8_t valid_press;
volatile int32_t last_position = 0;
volatile float measured_rpm = 0;
uint32_t counter = 0;

int16_t count = 0;

int16_t position = 0;

int speed =0;

void Start_Sensors_Task(void const * argument) {
	/*for (uint8_t addr = 1; addr < 127; addr++) {
		if (HAL_I2C_IsDeviceReady(&hi2c2, addr << 1, 2, 10) == HAL_OK) {
			char msg[64];
			snprintf(msg, sizeof(msg), "I2C device found at 0x%02X\r\n", addr);
			HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 1000);
		}
	}*/
/*	SensorReadings_t myData;
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
		HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 1000);
	}
	float T = 0, RH = 0;
	if (sht40_read_temperature_humidity(&T, &RH) == HAL_OK) {
		char buf[64];
		snprintf(buf, sizeof(buf), "Temp: %.2f °C, RH: %.2f %%\r\n", T, RH);
		HAL_UART_Transmit(&huart2, (uint8_t*)buf, strlen(buf), 1000);
	} else {
		HAL_UART_Transmit(&huart2, (uint8_t*)"Erreur capteur SHT40\r\n", 23, 1000);
	}
	*/
 HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);

	for (;;) {
		 Encoder_Update_Speed();

		    char msg[64];
		  //  snprintf(msg, sizeof(msg), "RPM: %.2f\r\n", measured_rpm);
		  //  HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);

		    osDelay(1000);

		/*if (valid_press){
			counter++;
			valid_press=0;
			if (counter>=10)
			{
				HAL_UART_Transmit(&huart2, (uint8_t*)"🌧️ Précipitation détectée!\r\n", strlen("🌧️ Précipitation détectée!\r\n"), 5000);
				counter = 0;

			}
			char buffer[50];
			snprintf(buffer, sizeof(buffer), "Counter = %d\r\n", counter);
			HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
		}*/


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
	if(GPIO_Pin==GPIO_PIN_1 && debounce_active == 0)
	{   debounce_active=1;
	__HAL_TIM_SET_COUNTER(&htim2, 0);
	HAL_TIM_Base_Start_IT(&htim2);
	}

}


/*void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	counter = __HAL_TIM_GET_COUNTER(&htim3);

	count = (int16_t)counter;

	position = count/4;
}*/
void Encoder_Update_Speed() {
    int32_t current_position = (int16_t)__HAL_TIM_GET_COUNTER(&htim3);
    int32_t delta = current_position - last_position;

    // handle overflow
    if (delta > 32768) delta -= 65536;
    if (delta < -32768) delta += 65536;

    last_position = current_position;

    float CPR = 330.0f;

    // ignorer les changements anormaux
    if (abs(delta) > 3000) {
        measured_rpm = 0;
        return;
    }

    measured_rpm = fabs((delta / CPR) * 600.0f);  // vitesse absolue
}
