/*
 * BH1750.h
 *
 *  Created on: Aug 18, 2025
 *      Author: 2023
 */

#ifndef INC_BH1750_H_
#define INC_BH1750_H_

#include "main.h"
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#define BH1750_ADDR      0x23 << 1   // ADDR = L

// Power / Reset
#define BH1750_POWER_DOWN   0x00
#define BH1750_POWER_ON     0x01
#define BH1750_RESET        0x07

// Continuously Measurement
#define BH1750_CONT_H_RES_MODE      0x10
#define BH1750_CONT_H_RES_MODE2     0x11
#define BH1750_CONT_L_RES_MODE      0x13

// One-time Measurement
#define BH1750_ONE_H_RES_MODE       0x20
#define BH1750_ONE_H_RES_MODE2      0x21
#define BH1750_ONE_L_RES_MODE       0x23



extern I2C_HandleTypeDef hi2c1;


HAL_StatusTypeDef BH1750_ResetSensor(void);

HAL_StatusTypeDef BH1750_PowerOn(void);

HAL_StatusTypeDef BH1750_PowerDown(void);

HAL_StatusTypeDef BH1750_SetMode(uint8_t mode);

HAL_StatusTypeDef BH1750_ReadRaw(uint16_t *raw);

float BH1750_CalcLux(uint16_t raw);

HAL_StatusTypeDef BH1750_ReadLux(uint8_t mode, float *lux);

HAL_StatusTypeDef BH1750_SetMeasurementTime(uint8_t mtreg);


#endif /* INC_BH1750_H_ */

