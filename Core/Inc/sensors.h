/*
 * sensors.h
 *
 *  Created on: Jul 10, 2025
 *      Author: ThinkPad
 */

#ifndef INC_SENSORS_H_
#define INC_SENSORS_H_
#include "main.h"
#include "adc.h"
#include "ihm.h"
#include "i2c.h"
#include "sht40_driver.h"
#include "main.h"
#include "cmsis_os.h"
#include <string.h>
#include "math.h"
#include "tim.h"
#define ADC_VREF 3.3f
#define ADC_RES 4095.0f
#define STABILITY_THRESHOLD 0.05f
#define MAX_ATTEMPTS 10
#define I2C_ADDR_SHT40 (0x44 << 1)
#define SHT40_CMD_MEASURE_HIGH 0xFD

uint8_t sht40_rx[6];
typedef struct {
    float soil_voltage;
    float light_voltage;
} SensorReadings_t;
int GetStableADCReading_Polling(SensorReadings_t *result);
void Encoder_Update_Speed();

#endif /* INC_SENSORS_H_ */
