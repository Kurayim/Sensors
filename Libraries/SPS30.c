/*
 * SPS30.c
 *
 *  Created on: Aug 16, 2025
 *      Author: 2023
 */

#include "SPS30.h"




uint8_t SPS30_CalcCRC(uint8_t *data, uint16_t length) {
    uint8_t crc = 0xFF;

    for(uint16_t i = 0; i < length; i++) {
        crc ^= data[i];
        for(uint8_t bit = 8; bit > 0; --bit) {
            if(crc & 0x80) {
                crc = (crc << 1) ^ 0x31u;  // Polynomial: 0x31
            } else {
                crc = (crc << 1);
            }
        }
    }

    return crc;
}

HAL_StatusTypeDef SPS30_DeviceReset(void){

    uint8_t buf[2];


    buf[0] = (SPS30_CMD_RESET >> 8) & 0xFF;  // MSB
    buf[1] = SPS30_CMD_RESET & 0xFF;         // LSB

    if(HAL_I2C_Master_Transmit(&hi2c1, SPS30_I2C_ADDR, buf, 2, I2C_TIMEOUT) != HAL_OK) {
        return HAL_ERROR;
    }

    HAL_Delay(100);
    return HAL_OK;
}

HAL_StatusTypeDef SPS30_StartFanCleaning(void) {
    uint8_t buf[2];

    buf[0] = (SPS30_CMD_START_FAN_CLEANING >> 8) & 0xFF;  // MSB
    buf[1] = SPS30_CMD_START_FAN_CLEANING & 0xFF;         // LSB

    if(HAL_I2C_Master_Transmit(&hi2c1, SPS30_I2C_ADDR, buf, 2, I2C_TIMEOUT) != HAL_OK) {
        return HAL_ERROR;
    }

    HAL_Delay(10000);

    return HAL_OK;
}

HAL_StatusTypeDef SPS30_WakeUp(void) {
    uint8_t buf[2];

    // Prepare the pointer bytes: MSB and LSB
    buf[0] = (SPS30_CMD_WAKEUP >> 8) & 0xFF;  // MSB
    buf[1] = SPS30_CMD_WAKEUP & 0xFF;         // LSB

    // First wake-up command: activates the I2C interface
    if(HAL_I2C_Master_Transmit(&hi2c1, SPS30_I2C_ADDR, buf, 2, I2C_TIMEOUT) != HAL_OK) {
        return HAL_ERROR;
    }
    // Short delay before sending the second command (optional)
    HAL_Delay(5);
    // Second wake-up command: sets sensor to Idle Mode
    if(HAL_I2C_Master_Transmit(&hi2c1, SPS30_I2C_ADDR, buf, 2, I2C_TIMEOUT) != HAL_OK) {
        return HAL_ERROR;
    }

    return HAL_OK;
}

HAL_StatusTypeDef SPS30_Sleep(void) {
    uint8_t buf[2];

    // Prepare pointer bytes for Sleep command (0x1001)
    buf[0] = (SPS30_CMD_SLEEP >> 8) & 0xFF;  // MSB
    buf[1] = SPS30_CMD_SLEEP & 0xFF;         // LSB

    // Send Sleep command via I2C
    if(HAL_I2C_Master_Transmit(&hi2c1, SPS30_I2C_ADDR, buf, 2, I2C_TIMEOUT) != HAL_OK) {
        return HAL_ERROR;  // Return error if transmission fails
    }

    return HAL_OK;  // Return OK if successful
}
