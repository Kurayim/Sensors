/*
 * BH1750.c
 *
 *  Created on: Aug 18, 2025
 *      Author: 2023
 */


#include "BH1750.h"

HAL_StatusTypeDef BH1750_ResetSensor(void){

    uint8_t cmd;
    HAL_StatusTypeDef ret;

    // 1. Power On
    cmd = BH1750_POWER_ON;
    ret = HAL_I2C_Master_Transmit(&hi2c1, BH1750_ADDR, &cmd, 1, HAL_MAX_DELAY);
    if(ret != HAL_OK) return ret;

    HAL_Delay(10);

    // 2. Reset
    cmd = BH1750_RESET;
    ret = HAL_I2C_Master_Transmit(&hi2c1, BH1750_ADDR, &cmd, 1, HAL_MAX_DELAY);

    return ret;
}


HAL_StatusTypeDef BH1750_PowerOn(void)
{
    uint8_t cmd = BH1750_POWER_ON;  // 0x01
    return HAL_I2C_Master_Transmit(&hi2c1, BH1750_ADDR, &cmd, 1, HAL_MAX_DELAY);
}


HAL_StatusTypeDef BH1750_PowerDown(void)
{
    uint8_t cmd = BH1750_POWER_DOWN;
    return HAL_I2C_Master_Transmit(&hi2c1, BH1750_ADDR, &cmd, 1, HAL_MAX_DELAY);
}

HAL_StatusTypeDef BH1750_SetMode(uint8_t mode)
{
    return HAL_I2C_Master_Transmit(&hi2c1, BH1750_ADDR, &mode, 1, HAL_MAX_DELAY);
}


HAL_StatusTypeDef BH1750_ReadRaw(uint16_t *raw)
{
    uint8_t data[2];
    HAL_StatusTypeDef ret;

    ret = HAL_I2C_Master_Receive(&hi2c1, BH1750_ADDR, data, 2, HAL_MAX_DELAY);
    if(ret != HAL_OK)
    	return ret;

    *raw = (data[0] << 8) | data[1];

    return HAL_OK;
}

float BH1750_CalcLux(uint16_t raw)
{
    return (float)raw / 1.2f; // Assuming MTreg default
}

HAL_StatusTypeDef BH1750_ReadLux(uint8_t mode, float *lux)
{
    HAL_StatusTypeDef ret;
    uint16_t raw;

    // 1. Power On
    ret = BH1750_PowerOn();
    if(ret != HAL_OK)
    	return ret;

    // 2. Set measurement mode
    ret = BH1750_SetMode(mode);
    if(ret != HAL_OK)
    	return ret;

    // 3. Wait for measurement to complete
    switch(mode)
    {
        case BH1750_CONT_H_RES_MODE:
        case BH1750_CONT_H_RES_MODE2:
            HAL_Delay(200); // 120ms typical + margin
            break;
        case BH1750_CONT_L_RES_MODE:
            HAL_Delay(30);  // 16ms typical + margin
            break;
        default:
            return HAL_ERROR; // invalid mode
    }

    // 4. Read raw data
    ret = BH1750_ReadRaw(&raw);
    if(ret != HAL_OK) return ret;

    // 5. Calculate Lux
    *lux = BH1750_CalcLux(raw);

    return HAL_OK;
}


HAL_StatusTypeDef BH1750_SetMeasurementTime(uint8_t mtreg)
{
    HAL_StatusTypeDef status;
    uint8_t high = 0x40 | (mtreg >> 5); // 01000_MT[7,6,5]
    uint8_t low  = 0x60 | (mtreg & 0x1F); // 011_MT[4,3,2,1,0]

    // send High byte
    status = HAL_I2C_Master_Transmit(&hi2c1, BH1750_ADDR, &high, 1, HAL_MAX_DELAY);
    if (status != HAL_OK) return status;

    // send Low byte
    status = HAL_I2C_Master_Transmit(&hi2c1, BH1750_ADDR, &low, 1, HAL_MAX_DELAY);
    return status;
}



