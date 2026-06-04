#include "ccpa_telemetry.h"
#include <string.h>

#define CONTROLLER_INFO00ACK 0x1E942040U
#define CONTROLLER_INFO00BRO 0x1E942042U
#define CONTROLLER_INFO03ACK 0x1E94204CU
#define CONTROLLER_INFO03BRO 0x1E94204EU

#define HMI_INFO00ACK 0x1E942C40U
#define HMI_INFO00BRO 0x1E942C42U
#define CENTRAL_INTERFACE_ASSISTREQ 0x2940015U

#define BAT1_INFO01ACK 0x1E942444U
#define BAT1_INFO01BRO 0x1E942446U
#define BAT1_INFO06ACK 0x1E942458U
#define BAT1_INFO06BRO 0x1E94245AU

#define REARDERAILLEUR_INFO00ACK 0x1E944840U
#define REARDERAILLEUR_INFO00BRO 0x1E944842U

static uint16_t ccpa_u16_le(const uint8_t *data)
{
    return (uint16_t)data[0] | ((uint16_t)data[1] << 8);
}

static uint32_t ccpa_u24_le(const uint8_t *data)
{
    return (uint32_t)data[0] | ((uint32_t)data[1] << 8) | ((uint32_t)data[2] << 16);
}

static int16_t ccpa_temp_c(uint8_t raw)
{
    return (int16_t)raw - 64;
}

static void ccpa_update_assist_level(CCPA_TelemetryState *state, uint8_t assistLevel, uint32_t *changed)
{
    if (assistLevel <= 5U) {
        state->assistLevel = assistLevel;
        state->assistLevelValid = true;
        *changed |= CCPA_TELEMETRY_CHANGED_ASSIST_LEVEL;
    } else if (assistLevel == 6U) {
        state->assistLevelValid = false;
    }
}

void ccpa_telemetry_init(CCPA_TelemetryState *state)
{
    if (state == 0) {
        return;
    }

    memset(state, 0, sizeof(*state));
}

uint32_t ccpa_telemetry_process_frame(CCPA_TelemetryState *state, const CanFrame *frame)
{
    if (state == 0 || frame == 0) {
        return CCPA_TELEMETRY_CHANGED_NONE;
    }

    uint32_t changed = CCPA_TELEMETRY_CHANGED_NONE;

    switch (frame->id) {
        case CCPA_GENERAL_INFO00ACK:
        case CCPA_GENERAL_INFO00BRO:
            if (frame->dlc >= 6) {
                uint16_t speedRaw = ccpa_u16_le(&frame->data[0]);
                if (speedRaw != 0xFFFFU) {
                    state->bikeSpeedKph = (float)speedRaw * 0.01f;
                    state->bikeSpeedValid = true;
                    state->bikeSpeedSource = CCPA_TELEMETRY_SOURCE_GENERAL_INFO;
                    changed |= CCPA_TELEMETRY_CHANGED_BIKE_SPEED;
                } else {
                    state->bikeSpeedValid = false;
                    state->bikeSpeedSource = CCPA_TELEMETRY_SOURCE_NONE;
                }

                state->cadenceRpm = frame->data[4];
                state->cadenceValid = true;
                state->cadenceSource = CCPA_TELEMETRY_SOURCE_GENERAL_INFO;
                changed |= CCPA_TELEMETRY_CHANGED_CADENCE;

                uint8_t assistLevel = (frame->data[5] >> 2) & 0x0FU;
                ccpa_update_assist_level(state, assistLevel, &changed);

            }
            break;

        case HMI_INFO00ACK:
        case HMI_INFO00BRO:
            if (frame->dlc >= 1) {
                ccpa_update_assist_level(state, frame->data[0] & 0x0FU, &changed);
            }
            break;

        case CENTRAL_INTERFACE_ASSISTREQ:
            if (frame->dlc >= 2) {
                ccpa_update_assist_level(state, frame->data[1] & 0x0FU, &changed);
            }
            break;

        case CCPA_GENERAL_INFO01ACK:
        case CCPA_GENERAL_INFO01BRO:
            if (frame->dlc >= 5) {
                if (frame->data[4] != 0xFFU) {
                    state->batterySocPct = frame->data[4];
                    state->batterySocValid = true;
                    state->batterySocSource = CCPA_TELEMETRY_SOURCE_GENERAL_INFO;
                    changed |= CCPA_TELEMETRY_CHANGED_BATTERY_SOC;
                } else {
                    state->batterySocValid = false;
                    state->batterySocSource = CCPA_TELEMETRY_SOURCE_NONE;
                }
            }
            break;

        case CONTROLLER_INFO00ACK:
        case CONTROLLER_INFO00BRO:
            if (frame->dlc >= 2 && state->bikeSpeedSource != CCPA_TELEMETRY_SOURCE_GENERAL_INFO) {
                uint16_t speedRaw = ccpa_u16_le(&frame->data[0]);
                if (speedRaw != 0xFFFFU) {
                    state->bikeSpeedKph = (float)speedRaw * 0.01f;
                    state->bikeSpeedValid = true;
                    state->bikeSpeedSource = CCPA_TELEMETRY_SOURCE_DEVICE;
                    changed |= CCPA_TELEMETRY_CHANGED_BIKE_SPEED;
                }
            }
            break;

        case CONTROLLER_INFO03ACK:
        case CONTROLLER_INFO03BRO:
            if (frame->dlc >= 4) {
                if (state->cadenceSource != CCPA_TELEMETRY_SOURCE_GENERAL_INFO) {
                    state->cadenceRpm = frame->data[3];
                    state->cadenceValid = true;
                    state->cadenceSource = CCPA_TELEMETRY_SOURCE_DEVICE;
                    changed |= CCPA_TELEMETRY_CHANGED_CADENCE;
                }
            }
            break;

        case BAT1_INFO01ACK:
        case BAT1_INFO01BRO:
            if (frame->dlc >= 7) {
                state->batteryVoltageMv = ccpa_u24_le(&frame->data[0]);
                state->batteryVoltageValid = true;
                changed |= CCPA_TELEMETRY_CHANGED_BATTERY_VOLTAGE;

                state->batteryCurrentMa = ccpa_u16_le(&frame->data[4]);
                state->batteryCurrentValid = true;
                changed |= CCPA_TELEMETRY_CHANGED_BATTERY_CURRENT;

                if (state->batterySocSource != CCPA_TELEMETRY_SOURCE_GENERAL_INFO && frame->data[6] != 0xFFU) {
                    state->batterySocPct = frame->data[6];
                    state->batterySocValid = true;
                    state->batterySocSource = CCPA_TELEMETRY_SOURCE_DEVICE;
                    changed |= CCPA_TELEMETRY_CHANGED_BATTERY_SOC;
                }
            }
            break;

        case BAT1_INFO06ACK:
        case BAT1_INFO06BRO:
            if (frame->dlc >= 4) {
                for (uint8_t i = 0; i < 4U; ++i) {
                    state->batteryTempsC[i] = ccpa_temp_c(frame->data[i]);
                    state->batteryTempsValid[i] = true;
                }
                changed |= CCPA_TELEMETRY_CHANGED_BATTERY_TEMPS;
            }
            break;

        case REARDERAILLEUR_INFO00ACK:
        case REARDERAILLEUR_INFO00BRO:
            if (frame->dlc >= 2) {
                state->rearGearIndex = frame->data[0];
                state->rearGearMax = frame->data[1];
                state->rearGearValid = true;
                state->rearGearSource = CCPA_TELEMETRY_SOURCE_DEVICE;
                changed |= CCPA_TELEMETRY_CHANGED_REAR_GEAR;
            }
            break;

        default:
            break;
    }

    return changed;
}
