/*
 * SPS30.h
 *
 *  Created on: Aug 16, 2025
 *      Author: 2023
 */

#ifndef INC_SPS30_H_
#define INC_SPS30_H_


#include "main.h"
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

extern I2C_HandleTypeDef hi2c1;

#define SPS30_I2C_ADDR		0x69
#define I2C_TIMEOUT		1000





/* SPS30 I2C Commands - Defines */

/* Start Measurement
   Starts the measurement process. The sensor must be in Idle-Mode before this command. */
#define SPS30_CMD_START_MEASUREMENT       0x0010  // Set Pointer & Write Data, 3 bytes, <20ms, FW v1.0

/* Stop Measurement
   Stops the measurement and returns the sensor to Idle-Mode. */
#define SPS30_CMD_STOP_MEASUREMENT        0x0104  // Set Pointer, <20ms, FW v1.0

/* Read Data-Ready Flag
   Poll to check if new measurement data is available.
   Response: 3 bytes including CRC */
#define SPS30_CMD_READ_DATA_READY_FLAG    0x0202  // Set Pointer & Read Data, FW v1.0

/* Read Measured Values
   Reads the latest measurement values and resets the Data-Ready Flag.
   Data format depends on output format: float (60 bytes) or integer (30 bytes) */
#define SPS30_CMD_READ_MEASURED_VALUES    0x0300  // Set Pointer & Read Data, FW v1.0

/* Sleep
   Puts the sensor into Sleep-Mode to minimize power consumption.
   I2C interface is disabled in this mode. */
#define SPS30_CMD_SLEEP                    0x1001  // Set Pointer, <5ms, FW v2.0

/* Wake-up
   Wakes up the sensor from Sleep-Mode to Idle-Mode.
   Can be sent twice if software cannot generate I2C Start/Stop pulse. */
#define SPS30_CMD_WAKEUP                   0x1103  // Set Pointer, <5ms, FW v2.0

/* Start Fan Cleaning
   Manually starts the fan cleaning procedure (10 seconds at maximum speed). */
#define SPS30_CMD_START_FAN_CLEANING       0x5607  // Set Pointer, <5ms, FW v1.0

/* Read/Write Auto Cleaning Interval
   Reads or sets the interval of automatic fan cleaning in seconds.
   Data format: 32-bit unsigned big-endian split into 2x16-bit + CRC */
#define SPS30_CMD_AUTO_CLEANING_INTERVAL   0x8004  // Set Pointer & Read/Write Data, read/write 6 bytes, FW v1.0/v2.2

/* Read Product Type
   Returns product type as ASCII string (e.g., "00080000"). */
#define SPS30_CMD_READ_PRODUCT_TYPE        0xD002  // Set Pointer & Read Data, 12 bytes, FW v1.0

/* Read Serial Number
   Returns the sensor's serial number as ASCII string (max 48 bytes). */
#define SPS30_CMD_READ_SERIAL_NUMBER       0xD033  // Set Pointer & Read Data, max 48 bytes, FW v1.0

/* Read Version
   Reads the sensor firmware version (3 bytes). */
#define SPS30_CMD_READ_VERSION             0xD100  // Set Pointer & Read Data, 3 bytes, FW v1.0

/* Read Device Status Register
   Returns 32-bit status flags including fan, laser, and errors. */
#define SPS30_CMD_READ_DEVICE_STATUS       0xD206  // Set Pointer & Read Data, 6 bytes, FW v2.2

/* Clear Device Status Register
   Clears status flags in the device. */
#define SPS30_CMD_CLEAR_DEVICE_STATUS      0xD210  // Set Pointer, <5ms, FW v2.0

/* Reset
   Resets the sensor module. */
#define SPS30_CMD_RESET                     0xD304  // Set Pointer, <100ms, FW v1.0






typedef enum
{
  SPS30_OK			= 0x00U,
  SPS30_ERROR  		= 0x01U,

} SPS30_StateTypeDef;

typedef struct
{
  uint32_t auto_clean_interval_s;  // Interval in seconds

} SPS30_AutoClean_t;

typedef struct {
    uint8_t major;
    uint8_t minor;
} SPS30_FirmwareVersion_t;

// Struct for float measurements
typedef struct {
    float pm1_0;
    float pm2_5;
    float pm4_0;
    float pm10;
    float nc0_5;
    float nc1_0;
    float nc2_5;
    float nc4_0;
    float nc10;
    float typical_size;
} SPS30_Measurement_Float_t;

// Struct for uint16 measurements
typedef struct {
    uint16_t pm1_0;
    uint16_t pm2_5;
    uint16_t pm4_0;
    uint16_t pm10;
    uint16_t nc0_5;
    uint16_t nc1_0;
    uint16_t nc2_5;
    uint16_t nc4_0;
    uint16_t nc10;
    uint16_t typical_size;
} SPS30_Measurement_U16_t;


/**
 * @brief Calculate CRC8 for an array of bytes (Sensirion CRC algorithm)
 * @param data Pointer to input data array
 * @param length Number of bytes in the array
 * @return CRC8 value
 */
uint8_t SPS30_CalcCRC(uint8_t *data, uint16_t length);


HAL_StatusTypeDef SPS30_DeviceReset(void);


HAL_StatusTypeDef SPS30_StartFanCleaning(void);


HAL_StatusTypeDef SPS30_WakeUp(void);


HAL_StatusTypeDef SPS30_Sleep(void);


HAL_StatusTypeDef SPS30_StopMeasurement(void);


HAL_StatusTypeDef SPS30_StartMeasurement(uint8_t format);


HAL_StatusTypeDef SPS30_WriteAutoCleaningInterval(uint32_t interval);


HAL_StatusTypeDef SPS30_ReadAutoCleaningInterval(uint32_t *interval);


HAL_StatusTypeDef SPS30_ReadDeviceInfo(uint16_t pointer, char *output, uint8_t max_len);


HAL_StatusTypeDef SPS30_GetProductType(char *product_type);


HAL_StatusTypeDef SPS30_GetSerialNumber(char *serial_number);







#endif /* INC_SPS30_H_ */
