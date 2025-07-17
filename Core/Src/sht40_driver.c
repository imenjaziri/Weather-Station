/*
 * sht40_driver.c
 *
 *  Created on: Jul 13, 2025
 *      Author: ThinkPad
 */

/* sht40_driver.c - STM32 HAL driver for SHT40 temperature & humidity sensor */

#include "sht40_driver.h"
#include "i2c.h"
#include <string.h>
#include "cmsis_os.h"
#define SHT40_I2C_ADDR         (0x44 << 1)  // Default I2C address for SHT40-AD1B
#define SHT40_CMD_MEASURE_HIGH 0xFD         // High precision measurement command

// Optional: CRC-8 polynomial and init for SHT4x
#define SHT40_CRC_POLYNOMIAL   0x31
#define SHT40_CRC_INIT         0xFF

static uint8_t sht40_crc8(const uint8_t *data, uint8_t len) {
    uint8_t crc = SHT40_CRC_INIT;
    for (uint8_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x80)
                crc = (crc << 1) ^ SHT40_CRC_POLYNOMIAL;
            else
                crc <<= 1;
        }
    }
    return crc;
}

HAL_StatusTypeDef sht40_read_temperature_humidity(float *temperature, float *humidity) {
    uint8_t cmd = SHT40_CMD_MEASURE_HIGH;
    uint8_t rx_buf[6] = {0};

    HAL_StatusTypeDef ret = HAL_I2C_Master_Transmit(&hi2c2, SHT40_I2C_ADDR, &cmd, 1, HAL_MAX_DELAY);
    if (ret != HAL_OK) return ret;

    osDelay(10); // wait for measurement to complete (~9.2 ms for high precision)

    ret = HAL_I2C_Master_Receive(&hi2c2, SHT40_I2C_ADDR, rx_buf, 6, HAL_MAX_DELAY);
    if (ret != HAL_OK) return ret;

    // Verify CRCs
    if (sht40_crc8(rx_buf, 2) != rx_buf[2]) return HAL_ERROR;
    if (sht40_crc8(rx_buf + 3, 2) != rx_buf[5]) return HAL_ERROR;

    uint16_t raw_temp = (rx_buf[0] << 8) | rx_buf[1];
    uint16_t raw_hum  = (rx_buf[3] << 8) | rx_buf[4];

    *temperature = -45.0f + 175.0f * ((float)raw_temp / 65535.0f);
    *humidity    = -6.0f + 125.0f * ((float)raw_hum / 65535.0f);

    return HAL_OK;
}

// Optional init function (does nothing here)
void sht40_init(I2C_HandleTypeDef *hi2c) {
    (void)hi2c;
}




