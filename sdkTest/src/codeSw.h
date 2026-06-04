#ifndef _CODE_SW_H_
#define _CODE_SW_H_

#include <Arduino.h>
#include "codeHw.h"

#define vTaskDelayMs(x) vTaskDelay(pdMS_TO_TICKS(x))

// --- 日誌系統定義 (Main / Hub) ---
#define DEBUG_MAIN_LEVEL 4
#define DEBUG_MAIN_SERIAL Serial

#if DEBUG_MAIN_LEVEL >= 1
#define MAIN_LOG_E(fmt, ...) \
    DEBUG_MAIN_SERIAL.printf("[E][%10lu][   ]%25s:%-5d > " fmt "\n", millis(), __FILE__, __LINE__, ##__VA_ARGS__);
#else
#define MAIN_LOG_E(fmt, ...) ;
#endif

#if DEBUG_MAIN_LEVEL >= 2
#define MAIN_LOG_W(fmt, ...) \
    DEBUG_MAIN_SERIAL.printf("[W][%10lu][   ]%25s:%-5d > " fmt "\n", millis(), __FILE__, __LINE__, ##__VA_ARGS__);
#else
#define MAIN_LOG_W(fmt, ...) ;
#endif

#if DEBUG_MAIN_LEVEL >= 3
#define MAIN_LOG_I(fmt, ...) \
    DEBUG_MAIN_SERIAL.printf("[I][%10lu][   ]%25s:%-5d > " fmt "\n", millis(), __FILE__, __LINE__, ##__VA_ARGS__);
#else
#define MAIN_LOG_I(fmt, ...) ;
#endif

#if DEBUG_MAIN_LEVEL >= 4
#define MAIN_LOG(fmt, ...) \
    DEBUG_MAIN_SERIAL.printf("[ ][%10lu][   ]%25s:%-5d > " fmt, millis(), __FILE__, __LINE__, ##__VA_ARGS__);
#define MAIN_LOG_S(fmt, ...) DEBUG_MAIN_SERIAL.printf(fmt, ##__VA_ARGS__);
#define MAIN_LOG_V(fmt, ...) \
    DEBUG_MAIN_SERIAL.printf("[V][%10lu][   ]%25s:%-5d > " fmt "\n", millis(), __FILE__, __LINE__, ##__VA_ARGS__);
#else
#define MAIN_LOG(fmt, ...)
#define MAIN_LOG_S(fmt, ...)
#define MAIN_LOG_V(fmt, ...) ;
#endif

#endif // _CODE_SW_H_
