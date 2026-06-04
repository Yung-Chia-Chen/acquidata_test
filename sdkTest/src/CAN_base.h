#ifndef _CAN_BASE_H_
#define _CAN_BASE_H_
#include <Arduino.h>
#ifdef CAN_lib_2
#include <ESP32CAN.h>
#endif
#define DEBUG_CAN_LEVEL 1 
#define DEBUG_CAN_SERIAL Serial

#if DEBUG_CAN_LEVEL >= 1

#define CAN_LOG_E(fmt, ...) DEBUG_CAN_SERIAL.printf("[E][%10lu][   ]%25s:%-5d > " fmt "\n", millis(), __FILE__, __LINE__, ##__VA_ARGS__);
#else
#define CAN_LOG_E(fmt, ...) ;
#endif

#if DEBUG_CAN_LEVEL >= 2
#define CAN_LOG_W(fmt, ...) DEBUG_CAN_SERIAL.printf("[W][%10lu][   ]%25s:%-5d > " fmt "\n", millis(), __FILE__, __LINE__, ##__VA_ARGS__);
#else
#define CAN_LOG_W(fmt, ...) ;
#endif

#if DEBUG_CAN_LEVEL >= 3
#define CAN_LOG_I(fmt, ...) DEBUG_CAN_SERIAL.printf("[I][%10lu][   ]%25s:%-5d > " fmt "\n", millis(), __FILE__, __LINE__, ##__VA_ARGS__);
#else
#define CAN_LOG_I(fmt, ...) ;
#endif

#if DEBUG_CAN_LEVEL >= 4
#define CAN_LOG(fmt, ...) DEBUG_CAN_SERIAL.printf("[ ][%10lu][   ]%25s:%-5d > " fmt, millis(), __FILE__, __LINE__, ##__VA_ARGS__);
#define CAN_LOG_S(fmt, ...) DEBUG_CAN_SERIAL.printf(fmt, ##__VA_ARGS__);
#define CAN_LOG_V(fmt, ...) DEBUG_CAN_SERIAL.printf("[V][%10lu][   ]%25s:%-5d > " fmt "\n", millis(), __FILE__, __LINE__, ##__VA_ARGS__);
#else
#define CAN_LOG(fmt, ...)
#define CAN_LOG_S(fmt, ...)
#define CAN_LOG_V(fmt, ...) ;

#endif
// -------------------------------------------------
// #define CAN_lib_1 // for ESP32 twai lib
// #define CAN_lib_2 // for https://github.com/sandeepmistry/arduino-CAN.git
#define vCANbus_baudrate 500000
typedef struct {
    union {
        struct {
            // The order of these bits must match deprecated message flags for compatibility reasons
            uint32_t extd : 1; /**< Extended Frame Format (29bit ID) */
            uint32_t rtr : 1; /**< Message is a Remote Frame */
            uint32_t ss : 1; /**< Transmit as a Single Shot Transmission. Unused for received. */
            uint32_t self : 1; /**< Transmit as a Self Reception Request. Unused for received. */
            uint32_t dlc_non_comp : 1; /**< Message's Data length code is larger than 8. This will break compliance with ISO 11898-1 */
            uint32_t reserved : 27; /**< Reserved bits */
        };
        // Todo: Deprecate flags
        uint32_t flags; /**< Deprecated: Alternate way to set bits using message flags */
    };
    uint32_t identifier; /**< 11 or 29 bit identifier */
    uint8_t data_length_code; /**< Data length code */
    uint8_t data[8]; /**< Data bytes (not relevant in RTR frame) */
} CAN_frame_t;
extern void CAN_base_setPin(int pinCanRx, int pinCanTx, long baudrate = vCANbus_baudrate);
extern bool CAN_base_init();
extern bool CAN_base_transmit(CAN_frame_t* data);
extern bool CAN_base_receive(CAN_frame_t* data, long timeout_ms);

#endif