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

HAL_StatusTypeDef SPS30_StopMeasurement(void){
    uint8_t buf[2];

    // Prepare pointer bytes for Stop Measurement command (0x0104)
    buf[0] = (SPS30_CMD_STOP_MEASUREMENT >> 8) & 0xFF;  // MSB
    buf[1] = SPS30_CMD_STOP_MEASUREMENT & 0xFF;         // LSB

    // Send Stop Measurement command via I2C
    if(HAL_I2C_Master_Transmit(&hi2c1, SPS30_I2C_ADDR, buf, 2, I2C_TIMEOUT) != HAL_OK) {
        return HAL_ERROR;  // Return error if transmission fails
    }

    return HAL_OK;  // Return OK if successful
}

HAL_StatusTypeDef SPS30_StartMeasurement(uint8_t format) {
    uint8_t buf[5];

    // Set Pointer (Command: 0x0010)
    buf[0] = (SPS30_CMD_START_MEASUREMENT >> 8) & 0xFF;  // MSB
    buf[1] = SPS30_CMD_START_MEASUREMENT & 0xFF;         // LSB

    // Write Data: Measurement format and dummy
    buf[2] = format;  // 0x03 for float or 0x05 for integer
    buf[3] = 0x00;    // dummy

    // Calculate CRC for bytes [2,3]
    buf[4] = SPS30_CalcCRC(&buf[2], 2);

    // Send command + data to SPS30
    if (HAL_I2C_Master_Transmit(&hi2c1, SPS30_I2C_ADDR, buf, 5, I2C_TIMEOUT) != HAL_OK) {
        return HAL_ERROR; // Transmission failed
    }

    return HAL_OK; // Successful
}


HAL_StatusTypeDef SPS30_ReadDataReady(uint8_t *ready) {
    uint8_t cmd[2] = {'\0'}; // Command: 0x0202
    uint8_t rxBuf[3] = {'\0'}; // 2 bytes data + 1 byte CRC


    cmd[0] = (SPS30_CMD_READ_DATA_READY_FLAG >> 8) & 0xFF;  // MSB
    cmd[1] = SPS30_CMD_READ_DATA_READY_FLAG & 0xFF;         // LSB


    // Send Set Pointer command
    if (HAL_I2C_Master_Transmit(&hi2c1, SPS30_I2C_ADDR, cmd, 2, I2C_TIMEOUT) != HAL_OK) {
        return HAL_ERROR;
    }

    // Read 3 bytes from sensor
    if (HAL_I2C_Master_Receive(&hi2c1, SPS30_I2C_ADDR, rxBuf, 3, I2C_TIMEOUT) != HAL_OK) {
        return HAL_ERROR;
    }

    // Verify CRC
    if (SPS30_CalcCRC(rxBuf, 2) != rxBuf[2]) {
        return HAL_ERROR; // CRC mismatch
    }

    // Extract Data-Ready flag (Byte 1)
    *ready = rxBuf[1];

    return HAL_OK;
}



HAL_StatusTypeDef SPS30_ReadMeasuredValues(bool isFloat, SPS30_Measurement_Float_t *float_data, SPS30_Measurement_U16_t *u16_data) {
    HAL_StatusTypeDef status;
    uint8_t cmd[2] = {'/0'}; // Pointer address 0x0300


    cmd[0] = (SPS30_CMD_READ_MEASURED_VALUES >> 8) & 0xFF;  // MSB
    cmd[1] =  SPS30_CMD_READ_MEASURED_VALUES & 0xFF;         // LSB


    // Send pointer command to sensor
    status = HAL_I2C_Master_Transmit(&hi2c1, SPS30_I2C_ADDR, cmd, 2, HAL_MAX_DELAY);
    if (status != HAL_OK)
    	return status;

    if (isFloat) {
        uint8_t rx_buf[60]; // 10 float values * 4 bytes + 2 CRC each
        status = HAL_I2C_Master_Receive(&hi2c1, SPS30_I2C_ADDR, rx_buf, sizeof(rx_buf), HAL_MAX_DELAY);
        if (status != HAL_OK)
        	return status;

        // Convert received bytes into floats
        for (int i = 0; i < 10; i++) {
            // Check CRC for first 2 bytes
            if (rx_buf[i*6 + 2] != SPS30_CalcCRC(&rx_buf[i*6], 2))
            	return HAL_ERROR;
            // Check CRC for next 2 bytes
            if (rx_buf[i*6 + 5] != SPS30_CalcCRC(&rx_buf[i*6 + 3], 2))
            	return HAL_ERROR;

            uint32_t temp = ((uint32_t)rx_buf[i*6] << 24) |
                            ((uint32_t)rx_buf[i*6 + 1] << 16) |
                            ((uint32_t)rx_buf[i*6 + 3] << 8) |
                            ((uint32_t)rx_buf[i*6 + 4]);
            memcpy(&((float*)float_data)[i], &temp, sizeof(float));
        }
    } else {
        uint8_t rx_buf[30]; // 10 uint16 values * 2 bytes + 1 CRC each
        status = HAL_I2C_Master_Receive(&hi2c1, SPS30_I2C_ADDR, rx_buf, sizeof(rx_buf), HAL_MAX_DELAY);
        if (status != HAL_OK)
        	return status;

        // Convert received bytes into uint16
        for (int i = 0; i < 10; i++) {
            // Check CRC
            if (rx_buf[i*3 + 2] != SPS30_CalcCRC(&rx_buf[i*3], 2))
            	return HAL_ERROR;

            ((uint16_t*)u16_data)[i] = ((uint16_t)rx_buf[i*3] << 8) | rx_buf[i*3 + 1];
        }
    }

    return HAL_OK;
}









HAL_StatusTypeDef SPS30_WriteAutoCleaningInterval(uint32_t interval) {
    HAL_StatusTypeDef status;
    uint8_t cmd[2] = {'\0'};
    uint8_t tx_buf[8] = {'\0'};

	// Pointer address
    cmd[0] = (SPS30_CMD_AUTO_CLEANING_INTERVAL >> 8) & 0xFF;  // MSB
    cmd[1] = SPS30_CMD_AUTO_CLEANING_INTERVAL & 0xFF;         // LSB

    // Convert 32-bit interval to Big-endian
    uint8_t data[4];
    data[0] = (interval >> 24) & 0xFF;
    data[1] = (interval >> 16) & 0xFF;
    data[2] = (interval >> 8) & 0xFF;
    data[3] = (interval) & 0xFF;

    // Fill buffer: two words + CRC for each
    tx_buf[0] = data[0];
    tx_buf[1] = data[1];
    tx_buf[2] = SPS30_CalcCRC(&data[0], 2);
    tx_buf[3] = data[2];
    tx_buf[4] = data[3];
    tx_buf[5] = SPS30_CalcCRC(&data[2], 2);

    // Send pointer first
    status = HAL_I2C_Master_Transmit(&hi2c1, SPS30_I2C_ADDR, cmd, 2, HAL_MAX_DELAY);
    if (status != HAL_OK)
    	return status;

    // Send data
    status = HAL_I2C_Master_Transmit(&hi2c1, SPS30_I2C_ADDR, tx_buf, 6, HAL_MAX_DELAY);
    return status;
}


HAL_StatusTypeDef SPS30_ReadAutoCleaningInterval(uint32_t *interval) {
    HAL_StatusTypeDef status;
    uint8_t cmd[2] = {'\0'};  // Pointer address
    uint8_t rx_buf[6] = {'\0'};

	// Pointer address
    cmd[0] = (SPS30_CMD_AUTO_CLEANING_INTERVAL >> 8) & 0xFF;  // MSB
    cmd[1] = SPS30_CMD_AUTO_CLEANING_INTERVAL & 0xFF;         // LSB

    // Send pointer
    status = HAL_I2C_Master_Transmit(&hi2c1, SPS30_I2C_ADDR, cmd, 2, HAL_MAX_DELAY);
    if (status != HAL_OK) return status;

    // Read 6 bytes (two words + CRC for each)
    status = HAL_I2C_Master_Receive(&hi2c1, SPS30_I2C_ADDR, rx_buf, 6, HAL_MAX_DELAY);
    if (status != HAL_OK) return status;

    // Check CRC for first two bytes
    if (rx_buf[2] != SPS30_CalcCRC(rx_buf, 2)) return HAL_ERROR;
    if (rx_buf[5] != SPS30_CalcCRC(&rx_buf[3], 2)) return HAL_ERROR;

    // Combine into 32-bit value (big-endian)
    *interval = ((uint32_t)rx_buf[0] << 24) | ((uint32_t)rx_buf[1] << 16) |
                ((uint32_t)rx_buf[3] << 8) | (uint32_t)rx_buf[4];

    return HAL_OK;
}


HAL_StatusTypeDef SPS30_ReadDeviceInfo(uint16_t pointer, char *output, uint8_t max_len) {
    HAL_StatusTypeDef status;
    uint8_t cmd[2] = { (pointer >> 8) & 0xFF, pointer & 0xFF };
    uint8_t rx_buf[48];  // Enough for max 32 chars + CRC bytes
    uint8_t index = 0;

    // Send pointer command
    status = HAL_I2C_Master_Transmit(&hi2c1, SPS30_I2C_ADDR, cmd, 2, HAL_MAX_DELAY);
    if (status != HAL_OK) return status;

    // Determine expected length (for serial number max 48 bytes)
    uint8_t expected_len = (max_len - 1) * 3 / 2;  // Each 2 chars + 1 CRC
    if (expected_len > sizeof(rx_buf))
    	expected_len = sizeof(rx_buf);

    // Read data
    status = HAL_I2C_Master_Receive(&hi2c1, SPS30_I2C_ADDR, rx_buf, expected_len, HAL_MAX_DELAY);
    if (status != HAL_OK)
    	return status;

    // Parse data: every 3 bytes → 2 chars + CRC
    for (uint8_t i = 0; i < expected_len; i += 3) {
        // Verify CRC for the two chars
        if (rx_buf[i+2] != SPS30_CalcCRC(&rx_buf[i], 2))
        	return HAL_ERROR;

        // Copy the two chars into output buffer
        if (index < (max_len - 1))
        	output[index++] = rx_buf[i];

        if ((index < (max_len - 1)) && (rx_buf[i+1] != 0))
        	output[index++] = rx_buf[i+1];
    }

    // Null-terminate the string
    output[index] = '\0';

    return HAL_OK;
}


HAL_StatusTypeDef SPS30_GetProductType(char *product_type) {
    return SPS30_ReadDeviceInfo(0xD002, product_type, 9);
}

HAL_StatusTypeDef SPS30_GetSerialNumber(char *serial_number) {
    return SPS30_ReadDeviceInfo(0xD033, serial_number, 33);
}


HAL_StatusTypeDef SPS30_ReadFirmwareVersion(SPS30_FirmwareVersion_t *fw_version) {
    HAL_StatusTypeDef status;
    uint8_t cmd[2] = {'\0'};      // Pointer address 0xD100
    uint8_t rx_buf[3] = {'\0'};  // 2 bytes data + 1 CRC

	// Pointer address
    cmd[0] = (SPS30_CMD_READ_VERSION >> 8) & 0xFF;  // MSB
    cmd[1] = SPS30_CMD_READ_VERSION & 0xFF;         // LSB

    // Send pointer command
    status = HAL_I2C_Master_Transmit(&hi2c1, SPS30_I2C_ADDR, cmd, 2, HAL_MAX_DELAY);
    if (status != HAL_OK) return status;

    // Read 3 bytes (2 data + CRC)
    status = HAL_I2C_Master_Receive(&hi2c1, SPS30_I2C_ADDR, rx_buf, 3, HAL_MAX_DELAY);
    if (status != HAL_OK) return status;

    // Check CRC
    if (rx_buf[2] != SPS30_CalcCRC(rx_buf, 2)) return HAL_ERROR;

    // Assign values
    fw_version->major = rx_buf[0];
    fw_version->minor = rx_buf[1];

    return HAL_OK;
}

HAL_StatusTypeDef SPS30_ReadDeviceStatus(uint32_t *device_status) {
    HAL_StatusTypeDef status;
    uint8_t cmd[2] = {'\0'};  	   // Pointer address 0xD206
    uint8_t rx_buf[6] = {'\0'};   // 4 bytes data + 2 CRCs

	// Pointer address
    cmd[0] = (SPS30_CMD_READ_DEVICE_STATUS >> 8) & 0xFF;  // MSB
    cmd[1] = SPS30_CMD_READ_DEVICE_STATUS & 0xFF;         // LSB

    // Send the pointer command to SPS30
    status = HAL_I2C_Master_Transmit(&hi2c1, SPS30_I2C_ADDR, cmd, 2, HAL_MAX_DELAY);
    if (status != HAL_OK)
    	return status;

    // Read 6 bytes (MSB 2 bytes + CRC + LSB 2 bytes + CRC)
    status = HAL_I2C_Master_Receive(&hi2c1, SPS30_I2C_ADDR, rx_buf, 6, HAL_MAX_DELAY);
    if (status != HAL_OK)
    	return status;

    // Check CRC for MSB
    if (rx_buf[2] != SPS30_CalcCRC(rx_buf, 2))
    	return HAL_ERROR;

    // Check CRC for LSB
    if (rx_buf[5] != SPS30_CalcCRC(&rx_buf[3], 2))
    	return HAL_ERROR;

    // Combine bytes into 32-bit status (big-endian)
    *device_status = ((uint32_t)rx_buf[0] << 24) |
                     ((uint32_t)rx_buf[1] << 16) |
                     ((uint32_t)rx_buf[3] << 8)  |
                     ((uint32_t)rx_buf[4]);

    return HAL_OK;
}








