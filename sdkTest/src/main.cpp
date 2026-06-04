#include <Arduino.h>
#include <string.h>
#include "s_canbus_spi.h"
#include "codeSw.h"
#include "CAN_base.h"
#include "ccpa_telemetry.h"
extern "C"
{
#include "chc_can_queue.h"
#include "chc_req_controller.h"
#include "chc_req_bat1.h"
}

CanFrame FrameRx;
CanFrame FrameRx_parse;
SPI_CAN_frame_t CAN_FrameRx;
CAN_frame_t twaiFrameRx;
S_CAN_HUB_INSTANCE spiQueue;
S_CAN_HUB_INSTANCE twaiQueue;

float fSpeed = 0.0;
uint32_t u32Odometer = 0;
uint16_t motorRPM = 0;
uint16_t u16ActuralCurrent = 0;
uint8_t u8BatteryPercentage = 0;
CCPA_TelemetryState telemetryState;

static constexpr uint32_t BATTERY_TEMP_REQUEST_INTERVAL_MS = 1000;
static uint32_t nextBatteryTempRequestMs = 0;

static const char *assistModeName(uint8_t assistLevel)
{
  static const char *const names[] = {"off", "eco", "eco+", "normal", "sport", "sport+"};
  return assistLevel < 6 ? names[assistLevel] : "unknown";
}

static bool decodeAssistRaw(uint32_t canId, const CanFrame *frame, uint8_t *rawValue, uint8_t *level)
{
  if (canId == CCPA_CENTRAL_INTERFACE_ASSISTREQ)
  {
    if (frame->dlc < 2)
    {
      return false;
    }
    *rawValue = frame->data[1];
    *level = *rawValue & 0x0F;
    return true;
  }
  return false;
}

static void logRawCanFrame(const char *busName, const CanFrame *frame)
{
  MAIN_LOG("RAW RX %s ID=0x%08lX DLC=%u DATA=", busName, (unsigned long)frame->id, frame->dlc);
  for (uint8_t i = 0; i < frame->dlc && i < 8; ++i)
  {
    MAIN_LOG_S("%02X ", frame->data[i]);
  }
  MAIN_LOG_S("\n");
}

static void logTelemetryUpdate(uint32_t canId, uint32_t changed)
{
  if (changed == CCPA_TELEMETRY_CHANGED_NONE)
  {
    return;
  }

  MAIN_LOG_I("CCPA telemetry update ID=0x%08lX changed=0x%08lX", (unsigned long)canId, (unsigned long)changed);

  if ((changed & CCPA_TELEMETRY_CHANGED_BIKE_SPEED) && telemetryState.bikeSpeedValid)
  {
    MAIN_LOG_I("Bike speed = %.2f KPH", telemetryState.bikeSpeedKph);
  }
  if ((changed & CCPA_TELEMETRY_CHANGED_CADENCE) && telemetryState.cadenceValid)
  {
    MAIN_LOG_I("Cadence = %u RPM", telemetryState.cadenceRpm);
  }
  if ((changed & CCPA_TELEMETRY_CHANGED_RIDER_TORQUE) && telemetryState.riderTorqueValid)
  {
    MAIN_LOG_I("Rider torque = %.2f Nm", telemetryState.riderTorqueNm);
  }
  if ((changed & CCPA_TELEMETRY_CHANGED_MOTOR_RPM) && telemetryState.motorRpmValid)
  {
    MAIN_LOG_I("Motor RPM = %u RPM", telemetryState.motorRpm);
  }
  if ((changed & CCPA_TELEMETRY_CHANGED_MOTOR_TEMPERATURE) && telemetryState.motorTemperatureValid)
  {
    MAIN_LOG_I("Motor temperature = %d C", (int)telemetryState.motorTemperatureC);
  }
  if ((changed & CCPA_TELEMETRY_CHANGED_ASSIST_LEVEL) && telemetryState.assistLevelValid)
  {
    MAIN_LOG_I("Assist mode = %s", assistModeName(telemetryState.assistLevel));
    static uint8_t lastAssistLevel = 0xFF;
    static bool assistLevelInitialized = false;
    if (!assistLevelInitialized || telemetryState.assistLevel != lastAssistLevel) {
      Serial.printf("ASSIST_CHANGE,%s\n", assistModeName(telemetryState.assistLevel));
      lastAssistLevel = telemetryState.assistLevel;
      assistLevelInitialized = true;
    }
  }
  else if (changed & CCPA_TELEMETRY_CHANGED_ASSIST_LEVEL)
  {
    MAIN_LOG_I("Assist mode = unknown");
  }
  if ((changed & CCPA_TELEMETRY_CHANGED_BATTERY_SOC) && telemetryState.batterySocValid)
  {
    MAIN_LOG_I("Battery SOC = %u %%", telemetryState.batterySocPct);
  }
  if ((changed & CCPA_TELEMETRY_CHANGED_BATTERY_VOLTAGE) && telemetryState.batteryVoltageValid)
  {
    MAIN_LOG_I("Battery voltage = %lu mV", (unsigned long)telemetryState.batteryVoltageMv);
  }
  if ((changed & CCPA_TELEMETRY_CHANGED_BATTERY_CURRENT) && telemetryState.batteryCurrentValid)
  {
    MAIN_LOG_I("Battery current = %u mA", telemetryState.batteryCurrentMa);
  }
  if (changed & CCPA_TELEMETRY_CHANGED_BATTERY_TEMPS)
  {
    MAIN_LOG_I("Battery temps = %d, %d, %d, %d C",
               telemetryState.batteryTempsValid[0] ? (int)telemetryState.batteryTempsC[0] : -999,
               telemetryState.batteryTempsValid[1] ? (int)telemetryState.batteryTempsC[1] : -999,
               telemetryState.batteryTempsValid[2] ? (int)telemetryState.batteryTempsC[2] : -999,
               telemetryState.batteryTempsValid[3] ? (int)telemetryState.batteryTempsC[3] : -999);
  }
  if ((changed & CCPA_TELEMETRY_CHANGED_REAR_GEAR) && telemetryState.rearGearValid)
  {
    MAIN_LOG_I("Rear gear = %u / %u", telemetryState.rearGearIndex, telemetryState.rearGearMax);
  }
}

static void requestBatteryTemps()
{
  const uint32_t now = millis();
  if (static_cast<int32_t>(now - nextBatteryTempRequestMs) < 0)
  {
    return;
  }
  nextBatteryTempRequestMs = now + BATTERY_TEMP_REQUEST_INTERVAL_MS;

  SPI_CAN_frame_t spiRequest{};
  spiRequest.ID = BAT1_INFO06REQ;
  spiRequest.ext = 1;
  spiRequest.rtr = 0;
  spiRequest.DLC = 8;
  canbus_spi_transmit(&spiRequest);

  CAN_frame_t twaiRequest{};
  twaiRequest.extd = 1;
  twaiRequest.rtr = 0;
  twaiRequest.identifier = BAT1_INFO06REQ;
  twaiRequest.data_length_code = 8;
  CAN_base_transmit(&twaiRequest);
}

void setup()
{
  Serial.begin(115200);
  CAN_base_setPin(pinMcu_CANBUS_RX_2, pinMcu_CANBUS_TX_2, 250000);
  initCanQueue();
  vCanQueue_initInstance(&spiQueue);
  vCanQueue_initInstance(&twaiQueue);
  ccpa_telemetry_init(&telemetryState);
  while(!canbus_spi_init())
  {
    MAIN_LOG_E("SPI CAN init failed");
    vTaskDelayMs(500);
  }
  
  while (!CAN_base_init())
  {
    MAIN_LOG_E("CAN init failed");
    vTaskDelayMs(500);
  }
}

void loop()
{
  requestBatteryTemps();

  if (canbus_spi_receive(&CAN_FrameRx, 5))
  {
    if ((CAN_FrameRx.ID & 0x80000000) == 0x80000000)
    {
      FrameRx.id = CAN_FrameRx.ID & 0x1FFFFFFF;
    }
    else
    {
      FrameRx.id = CAN_FrameRx.ID;
    }
    FrameRx.dlc = CAN_FrameRx.DLC > 8 ? 8 : CAN_FrameRx.DLC;
    memcpy(FrameRx.data, CAN_FrameRx.data, FrameRx.dlc);
    logRawCanFrame("SPI", &FrameRx);
    vCanQueue_enqueue(&spiQueue.sRxQueue, &FrameRx);
  }
  while (vCanQueue_dequeue(&spiQueue.sRxQueue, &FrameRx_parse) != 0)
  {
    uint8_t assistRaw = 0;
    uint8_t assistLevel = 0;
    if (decodeAssistRaw(FrameRx_parse.id, &FrameRx_parse, &assistRaw, &assistLevel))
    {
      MAIN_LOG_I("Assist raw ID=0x%08lX raw=0x%02X level=%u mode=%s",
                 (unsigned long)FrameRx_parse.id,
                 assistRaw,
                 assistLevel,
                 assistModeName(assistLevel));
    }
    logTelemetryUpdate(FrameRx_parse.id, ccpa_telemetry_process_frame(&telemetryState, &FrameRx_parse));

    if (controller_processQueueRX_ACKBRO(FrameRx_parse) != 0) // 如果不等於0, 代表這包是由controller傳出
    {
      fSpeed = (float)Controller_Info00.BikeSpeed * 0.01;
      u32Odometer = Controller_Info00.OdoMeter;
      motorRPM = Controller_Info02.MotorRPM;
      MAIN_LOG_I("Speed = %f, Odo = %d", fSpeed, u32Odometer);
      Serial.printf("motor rpm = %d", motorRPM);
    }
    else
    {

      // MAIN_LOG_E("非控制器訊號,ID = %2x",FrameRx_parse.id);
    }
  }

  if (CAN_base_receive(&twaiFrameRx, 5))
  {
    FrameRx.id = twaiFrameRx.identifier;
    FrameRx.dlc = twaiFrameRx.data_length_code > 8 ? 8 : twaiFrameRx.data_length_code;
    memcpy(FrameRx.data, twaiFrameRx.data, FrameRx.dlc);
    logRawCanFrame("TWAI", &FrameRx);
    uint8_t assistRaw = 0;
    uint8_t assistLevel = 0;
    if (decodeAssistRaw(FrameRx.id, &FrameRx, &assistRaw, &assistLevel))
    {
      MAIN_LOG_I("Assist raw ID=0x%08lX raw=0x%02X level=%u mode=%s",
                 (unsigned long)FrameRx.id,
                 assistRaw,
                 assistLevel,
                 assistModeName(assistLevel));
    }
    logTelemetryUpdate(FrameRx.id, ccpa_telemetry_process_frame(&telemetryState, &FrameRx));

    u16ActuralCurrent = telemetryState.batteryCurrentValid ? telemetryState.batteryCurrentMa : 0;
    u8BatteryPercentage = telemetryState.batterySocValid ? telemetryState.batterySocPct : 0;
  }

  vTaskDelayMs(1); // 迴圈結束後延遲1ms
}
