#include "imu_logger.h"

#include <Wire.h>
#include <math.h>

namespace
{
constexpr uint8_t MPU6050_ADDR_LOW = 0x68;
constexpr uint8_t MPU6050_ADDR_HIGH = 0x69;

constexpr uint8_t REG_SMPLRT_DIV = 0x19;
constexpr uint8_t REG_CONFIG = 0x1A;
constexpr uint8_t REG_GYRO_CONFIG = 0x1B;
constexpr uint8_t REG_ACCEL_CONFIG = 0x1C;
constexpr uint8_t REG_INT_ENABLE = 0x38;
constexpr uint8_t REG_ACCEL_XOUT_H = 0x3B;
constexpr uint8_t REG_PWR_MGMT_1 = 0x6B;
constexpr uint8_t REG_WHO_AM_I = 0x75;

constexpr float ACCEL_LSB_PER_G = 4096.0f;    // +/- 8 g
constexpr float GYRO_LSB_PER_DPS = 65.5f;     // +/- 500 deg/s
constexpr uint32_t SAMPLE_INTERVAL_US = 10000; // 100 Hz
constexpr uint16_t ACCEL_CALIBRATION_SAMPLES = 200;
constexpr uint16_t GYRO_CALIBRATION_SAMPLES = 200;

bool ready = false;
uint8_t imuAddress = MPU6050_ADDR_LOW;
uint32_t nextSampleUs = 0;
float gyroBiasX = 0.0f;
float gyroBiasY = 0.0f;
float gyroBiasZ = 0.0f;
float accelBiasX = 0.0f;
float accelBiasY = 0.0f;
float accelBiasZ = 0.0f;

struct ImuRaw
{
  int16_t ax;
  int16_t ay;
  int16_t az;
  int16_t temp;
  int16_t gx;
  int16_t gy;
  int16_t gz;
};

int16_t readI16(const uint8_t *data)
{
  return static_cast<int16_t>((static_cast<uint16_t>(data[0]) << 8) | data[1]);
}

bool writeRegister(uint8_t reg, uint8_t value)
{
  Wire.beginTransmission(imuAddress);
  Wire.write(reg);
  Wire.write(value);
  return Wire.endTransmission() == 0;
}

bool readRegisters(uint8_t reg, uint8_t *buffer, size_t length)
{
  Wire.beginTransmission(imuAddress);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0)
  {
    return false;
  }

  const size_t received = Wire.requestFrom(static_cast<int>(imuAddress), static_cast<int>(length));
  if (received != length)
  {
    return false;
  }

  for (size_t i = 0; i < length; ++i)
  {
    buffer[i] = Wire.read();
  }
  return true;
}

bool probeAddress(uint8_t address)
{
  Wire.beginTransmission(address);
  if (Wire.endTransmission() != 0)
  {
    return false;
  }

  imuAddress = address;
  uint8_t whoAmI = 0;
  if (!readRegisters(REG_WHO_AM_I, &whoAmI, 1))
  {
    return false;
  }

  // MPU6050 normally returns 0x68. Some compatible boards return 0x70.
  return whoAmI == 0x68 || whoAmI == 0x70;
}

bool readRaw(ImuRaw &raw)
{
  uint8_t buffer[14] = {0};
  if (!readRegisters(REG_ACCEL_XOUT_H, buffer, sizeof(buffer)))
  {
    return false;
  }

  raw.ax = readI16(&buffer[0]);
  raw.ay = readI16(&buffer[2]);
  raw.az = readI16(&buffer[4]);
  raw.temp = readI16(&buffer[6]);
  raw.gx = readI16(&buffer[8]);
  raw.gy = readI16(&buffer[10]);
  raw.gz = readI16(&buffer[12]);
  return true;
}

bool configureSensor()
{
  if (!writeRegister(REG_PWR_MGMT_1, 0x80))
  {
    return false;
  }
  delay(100);

  return writeRegister(REG_PWR_MGMT_1, 0x01) &&  // Wake up, use X gyro clock
         writeRegister(REG_CONFIG, 0x03) &&      // DLPF about 44 Hz
         writeRegister(REG_SMPLRT_DIV, 9) &&     // 100 Hz when DLPF is enabled
         writeRegister(REG_GYRO_CONFIG, 0x08) && // +/- 500 deg/s
         writeRegister(REG_ACCEL_CONFIG, 0x10) && // +/- 8 g
         writeRegister(REG_INT_ENABLE, 0x00);
}

void calibrateGyro()
{
  int64_t sumX = 0;
  int64_t sumY = 0;
  int64_t sumZ = 0;
  uint16_t count = 0;

  Serial.println(F("# Keep the bike/sensor still for gyro calibration"));
  for (uint16_t i = 0; i < GYRO_CALIBRATION_SAMPLES; ++i)
  {
    ImuRaw raw;
    if (readRaw(raw))
    {
      sumX += raw.gx;
      sumY += raw.gy;
      sumZ += raw.gz;
      ++count;
    }
    delay(5);
  }

  if (count > 0)
  {
    gyroBiasX = static_cast<float>(sumX) / static_cast<float>(count);
    gyroBiasY = static_cast<float>(sumY) / static_cast<float>(count);
    gyroBiasZ = static_cast<float>(sumZ) / static_cast<float>(count);
  }
}

void calibrateAccel()
{
  int64_t sumX = 0;
  int64_t sumY = 0;
  int64_t sumZ = 0;
  uint16_t count = 0;

  Serial.println(F("# Keep the bike/sensor still for accel calibration"));
  for (uint16_t i = 0; i < ACCEL_CALIBRATION_SAMPLES; ++i)
  {
    ImuRaw raw;
    if (readRaw(raw))
    {
      sumX += raw.ax;
      sumY += raw.ay;
      sumZ += raw.az;
      ++count;
    }
    delay(5);
  }

  if (count == 0)
  {
    return;
  }

  const float avgX = static_cast<float>(sumX) / static_cast<float>(count) / ACCEL_LSB_PER_G;
  const float avgY = static_cast<float>(sumY) / static_cast<float>(count) / ACCEL_LSB_PER_G;
  const float avgZ = static_cast<float>(sumZ) / static_cast<float>(count) / ACCEL_LSB_PER_G;
  const float norm = sqrtf(avgX * avgX + avgY * avgY + avgZ * avgZ);

  if (norm > 0.01f)
  {
    const float scale = 1.0f / norm;
    accelBiasX = avgX * (1.0f - scale);
    accelBiasY = avgY * (1.0f - scale);
    accelBiasZ = avgZ * (1.0f - scale);
  }
}
}

bool imu_logger_begin(uint8_t sdaPin, uint8_t sclPin, uint32_t i2cFrequencyHz)
{
  ready = false;
  Wire.begin(sdaPin, sclPin);
  Wire.setClock(i2cFrequencyHz);
  delay(100);

  if (!probeAddress(MPU6050_ADDR_LOW) && !probeAddress(MPU6050_ADDR_HIGH))
  {
    Serial.println(F("# MPU6050 not found on I2C address 0x68 or 0x69"));
    return false;
  }

  if (!configureSensor())
  {
    Serial.println(F("# MPU6050 configuration failed"));
    return false;
  }

  calibrateAccel();
  calibrateGyro();
  Serial.println(F("type,t_ms,ax_g,ay_g,az_g,gx_dps,gy_dps,gz_dps,temp_c,accel_norm_g"));

  ready = true;
  nextSampleUs = micros();
  return true;
}

void imu_logger_update()
{
  if (!ready)
  {
    return;
  }

  const uint32_t nowUs = micros();
  if (static_cast<int32_t>(nowUs - nextSampleUs) < 0)
  {
    return;
  }
  nextSampleUs += SAMPLE_INTERVAL_US;

  ImuRaw raw;
  if (!readRaw(raw))
  {
    Serial.println(F("# MPU6050 read failed"));
    return;
  }

  const float axG = static_cast<float>(raw.ax) / ACCEL_LSB_PER_G - accelBiasX;
  const float ayG = static_cast<float>(raw.ay) / ACCEL_LSB_PER_G - accelBiasY;
  const float azG = static_cast<float>(raw.az) / ACCEL_LSB_PER_G - accelBiasZ;
  const float gxDps = (static_cast<float>(raw.gx) - gyroBiasX) / GYRO_LSB_PER_DPS;
  const float gyDps = (static_cast<float>(raw.gy) - gyroBiasY) / GYRO_LSB_PER_DPS;
  const float gzDps = (static_cast<float>(raw.gz) - gyroBiasZ) / GYRO_LSB_PER_DPS;
  const float tempC = static_cast<float>(raw.temp) / 340.0f + 36.53f;
  const float accelNormG = sqrtf(axG * axG + ayG * ayG + azG * azG);

  Serial.print(F("IMU,"));
  Serial.print(millis());
  Serial.print(',');
  Serial.print(axG, 5);
  Serial.print(',');
  Serial.print(ayG, 5);
  Serial.print(',');
  Serial.print(azG, 5);
  Serial.print(',');
  Serial.print(gxDps, 3);
  Serial.print(',');
  Serial.print(gyDps, 3);
  Serial.print(',');
  Serial.print(gzDps, 3);
  Serial.print(',');
  Serial.print(tempC, 2);
  Serial.print(',');
  Serial.println(accelNormG, 5);
}

bool imu_logger_is_ready()
{
  return ready;
}
