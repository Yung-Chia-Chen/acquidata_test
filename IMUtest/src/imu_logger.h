#ifndef IMU_LOGGER_H
#define IMU_LOGGER_H

#include <Arduino.h>

bool imu_logger_begin(uint8_t sdaPin, uint8_t sclPin, uint32_t i2cFrequencyHz = 400000);
void imu_logger_update();
bool imu_logger_is_ready();

#endif
