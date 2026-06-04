#ifndef _CODEHW_H_
#define _CODEHW_H_

#include "codeSw.h"

// ? ===========================================================================
// ? Project: chcCANBridge (ESP32-C6-Mini-N4)
// ? ===========================================================================
#define pinMcu_VHIN_ADC 1
#define pinMcu_UART_TX_1 4
#define pinMcu_UART_RX_1 5
#define pinMcu_CANBUS_TX_2 8  // Internal CAN (TWAI) TX
#define pinMcu_BOOT 9
#define pinMcu_USB_D_MINUS 12
#define pinMcu_USB_D_PLUS 13
#define pinMcu_UART_TX_2 14
#define pinMcu_UART_RX_2 15
#define pinMcu_CANBUS_RX_2 18      // Internal CAN (TWAI) RX
#define pinMcu_SPI_CANBUS_CS_1 19  // MCP2515 CS
#define pinMcu_SPI_MISO 20         // MCP2515 MISO
#define pinMcu_SPI_MOSI 21         // MCP2515 MOSI
#define pinMcu_SPI_SCK 22          // MCP2515 SCK
#define pinMcu_CANBUS_INT_1 23     // MCP2515 INT
#define pinMcu_IMU_SDA 21          // MPU6050 I2C SDA (classic ESP32 default)
#define pinMcu_IMU_SCL 22          // MPU6050 I2C SCL (classic ESP32 default)

#define pinMcu_PWR_EN_Controller 14
#define pinMcu_PWR_EN_Device 15
// ? ===========================================================================
// ? ===========================================================================
// ? debug
#define Debug Serial
#define Debug_Baudrate 115200

#endif
