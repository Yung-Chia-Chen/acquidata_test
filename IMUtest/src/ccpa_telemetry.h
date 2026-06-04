#ifndef _CCPA_TELEMETRY_H_
#define _CCPA_TELEMETRY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include "chc_can_queue.h"

#define CCPA_GENERAL_INFO00ACK 0x29404FCU
#define CCPA_GENERAL_INFO00BRO 0x29404FEU
#define CCPA_GENERAL_INFO01ACK 0x29404F8U
#define CCPA_GENERAL_INFO01BRO 0x29404FAU

typedef enum {
    CCPA_TELEMETRY_SOURCE_NONE = 0,
    CCPA_TELEMETRY_SOURCE_GENERAL_INFO = 1,
    CCPA_TELEMETRY_SOURCE_DEVICE = 2
} CCPA_TelemetrySource;

typedef enum {
    CCPA_TELEMETRY_CHANGED_NONE = 0,
    CCPA_TELEMETRY_CHANGED_BIKE_SPEED = 1U << 0,
    CCPA_TELEMETRY_CHANGED_CADENCE = 1U << 1,
    CCPA_TELEMETRY_CHANGED_ASSIST_LEVEL = 1U << 2,
    CCPA_TELEMETRY_CHANGED_BATTERY_SOC = 1U << 3,
    CCPA_TELEMETRY_CHANGED_BATTERY_VOLTAGE = 1U << 4,
    CCPA_TELEMETRY_CHANGED_BATTERY_CURRENT = 1U << 5,
    CCPA_TELEMETRY_CHANGED_BATTERY_TEMPS = 1U << 6,
    CCPA_TELEMETRY_CHANGED_REAR_GEAR = 1U << 7
} CCPA_TelemetryChanged;

typedef struct {
    bool bikeSpeedValid;
    float bikeSpeedKph;
    CCPA_TelemetrySource bikeSpeedSource;

    bool cadenceValid;
    uint8_t cadenceRpm;
    CCPA_TelemetrySource cadenceSource;

    bool assistLevelValid;
    uint8_t assistLevel;

    bool batterySocValid;
    uint8_t batterySocPct;
    CCPA_TelemetrySource batterySocSource;

    bool batteryVoltageValid;
    uint32_t batteryVoltageMv;

    bool batteryCurrentValid;
    uint16_t batteryCurrentMa;

    bool batteryTempsValid[4];
    int16_t batteryTempsC[4];

    bool rearGearValid;
    uint8_t rearGearIndex;
    uint8_t rearGearMax;
    CCPA_TelemetrySource rearGearSource;
} CCPA_TelemetryState;

void ccpa_telemetry_init(CCPA_TelemetryState *state);
uint32_t ccpa_telemetry_process_frame(CCPA_TelemetryState *state, const CanFrame *frame);

#ifdef __cplusplus
}
#endif

#endif
