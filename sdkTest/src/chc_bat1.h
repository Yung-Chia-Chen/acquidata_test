#ifndef CHC_BAT1_H
#define CHC_BAT1_H

#include <stdint.h>
#include <string.h>
#include "chc_can_queue.h"
/* Cycle Times (ms) */
#define BAT1_BASICINFO00_CYCLE_TIME 3000
#define BAT1_BASICINFO01_CYCLE_TIME 3000
#define BAT1_BASICINFO02_CYCLE_TIME 3000
#define BAT1_BASICINFO03_CYCLE_TIME 5000
#define BAT1_INFO00_CYCLE_TIME 100
#define BAT1_INFO01_CYCLE_TIME 100
#define BAT1_INFO02_CYCLE_TIME 3000
#define BAT1_INFO03_CYCLE_TIME 5000
#define BAT1_INFO04_CYCLE_TIME 5000
#define BAT1_INFO05_CYCLE_TIME 5000
#define BAT1_INFO06_CYCLE_TIME 10000
#define BAT1_INFO07_CYCLE_TIME 3000
#define BAT1_ERROR_CYCLE_TIME 1000

/* CAN message IDs */
#define BAT1_INFO07REQ 0x1E94245DU
#define BAT1_INFO07BRO 0x1E94245EU
#define BAT1_INFO07ACK 0x1E94245CU
#define BAT1_INFO06REQ 0x1E942459U
#define BAT1_INFO06BRO 0x1E94245AU
#define BAT1_INFO06ACK 0x1E942458U
#define BAT1_INFO05REQ 0x1E942455U
#define BAT1_INFO05BRO 0x1E942456U
#define BAT1_INFO05ACK 0x1E942454U
#define BAT1_INFO04REQ 0x1E942451U
#define BAT1_INFO04BRO 0x1E942452U
#define BAT1_INFO04ACK 0x1E942450U
#define BAT1_INFO03REQ 0x1E94244DU
#define BAT1_INFO03BRO 0x1E94244EU
#define BAT1_INFO03ACK 0x1E94244CU
#define BAT1_INFO02REQ 0x1E942449U
#define BAT1_INFO02BRO 0x1E94244AU
#define BAT1_INFO02ACK 0x1E942448U
#define BAT1_INFO01REQ 0x1E942445U
#define BAT1_INFO01BRO 0x1E942446U
#define BAT1_INFO01ACK 0x1E942444U
#define BAT1_INFO00REQ 0x1E942441U
#define BAT1_INFO00BRO 0x1E942442U
#define BAT1_INFO00ACK 0x1E942440U
#define BAT1_BASICCOMMAND1REQ 0x1E942439U
#define BAT1_BASICCOMMAND1BRO 0x1E942438U
#define BAT1_BASICCOMMAND0REQ 0x1E942435U
#define BAT1_BASICCOMMAND0BRO 0x1E942434U
#define BAT1_BASICINFO03REQ 0x1E942431U
#define BAT1_BASICINFO03BRO 0x1E942432U
#define BAT1_BASICINFO03ACK 0x1E942430U
#define BAT1_BASICINFO02REQ 0x1E94242DU
#define BAT1_BASICINFO02BRO 0x1E94242EU
#define BAT1_BASICINFO02ACK 0x1E94242CU
#define BAT1_BASICINFO01REQ 0x1E942429U
#define BAT1_BASICINFO01BRO 0x1E94242AU
#define BAT1_BASICINFO01ACK 0x1E942428U
#define BAT1_BASICINFO00REQ 0x1E942425U
#define BAT1_BASICINFO00BRO 0x1E942426U
#define BAT1_BASICINFO00ACK 0x1E942424U
#define BAT1_MCUOPERATIONACK 0x1E94240DU
#define BAT1_MCUOPERATIONREQ 0x1E94240CU
#define BAT1_DEBUGMODEACK 0x1E942480U
#define BAT1_DEBUGMODEREQ 0x1E942481U
#define BAT1_ERRORREQ 0x1E9424BDU
#define BAT1_ERRORACK 0x1E9424BCU
#define BAT1_ERRORBRO 0x1E9424BCU
#define BAT1_WARNINGBRO 0x1E9424B8U
#define BAT1_TESTMODEACK 0x1E94243CU
#define BAT1_TESTMODEREQ 0x1E94243DU

/* Structure definitions */
typedef struct {
    // Signals
    uint8_t OperationCode;  // None
} BAT1_BASICCOMMAND0_REQ_T;

typedef struct {
    // Signals
    uint8_t reserved[8];
} BAT1_BASICCOMMAND0_BRO_T;

typedef struct {
    // Signals
    uint8_t reserved[8];
} BAT1_BASICCOMMAND1_REQ_T;

typedef struct {
    // Signals
    uint8_t reserved[8];
} BAT1_BASICCOMMAND1_BRO_T;

typedef struct {
    // Signals
    uint16_t BroadcastPeriodChange;
    uint8_t reserved[6];
} BAT1_BASICINFO00_REQ_T;

typedef struct {
    // Signals
    uint8_t MID[6];
    uint8_t BikeModelVersionASCII;
    uint8_t BikeModelVersionBCD;
} BAT1_BASICINFO00_ACK_T;

typedef struct {
    // Signals
    uint16_t BroadcastPeriodChange;
    uint8_t reserved[6];
} BAT1_BASICINFO01_REQ_T;

typedef struct {
    // Signals
    uint8_t FWVersionASCII;
    uint8_t FWVersionBCD;
    uint8_t HWVersionASCII;
    uint8_t HWVersionBCD;
    uint8_t KernelVersionASCII;
    uint8_t KernelVersionBCD;
    uint8_t BootloaderVersionASCII;
    uint8_t BootloaderVersionBCD;
} BAT1_BASICINFO01_ACK_T;

typedef struct {
    // Signals
    uint16_t BroadcastPeriodChange;
    uint8_t reserved[6];
} BAT1_BASICINFO02_REQ_T;

typedef struct {
    // Signals
    uint16_t LockConfiguration;
    uint8_t RegistrationStatus;
    uint8_t ApplicationFlag;
    uint8_t SupportProtocolType;
    uint8_t SupportProtocolVersionBCD;
    uint8_t SupportProtocolVersionASCII;
    uint8_t reserved;
} BAT1_BASICINFO02_ACK_T;

typedef struct {
    // Signals
    uint16_t BroadcastPeriodChange;
    uint8_t reserved[6];
} BAT1_BASICINFO03_REQ_T;

typedef struct {
    // Signals
    uint8_t BikeID[6];
    uint8_t reserved[2];
} BAT1_BASICINFO03_ACK_T;

typedef struct {
    // Signals
    uint8_t FromWho : 2;         // None
    uint8_t SequenceNumber : 2;  // None
    uint8_t reserved : 4;
    uint8_t DebugInfoNumber;  // None
    uint16_t TimeInterval;    // [ms]
    uint8_t Counter;          // None
} BAT1_DEBUGMODE_REQ_T;

typedef struct {
    // Signals
    uint8_t FromWho : 2;         // None
    uint8_t SequenceNumber : 2;  // None
    uint8_t ResponseCode : 4;    // None
} BAT1_DEBUGMODE_ACK_T;

typedef struct {
    // Signals
} BAT1_ERROR_REQ_T;

typedef struct {
    // Signals
    uint8_t TemperatureChargingError : 1;
    uint8_t TemperatureDischargingError : 1;
    uint8_t CellOverVoltage : 1;
    uint8_t CellUnderVoltage : 1;
    uint8_t OverCurrentCharge : 1;
    uint8_t ShortCircuit : 1;
    uint8_t OverCurrentDischarge : 1;
    uint8_t BatteryImbalancedCellVoltage : 1;
    uint8_t FrontLightMalfunction : 1;
    uint8_t RearLightMalfunction : 1;
    uint8_t LightShortCircuit : 1;
    uint8_t Reserved : 5;
    uint8_t Reserved2[6];
} BAT1_ERROR_ACK_T;

typedef struct {
    // Signals
    uint8_t Reserved[8];
} BAT1_WARNING_BRO_T;

typedef struct {
    // Signals
} BAT1_INFO00_REQ_T;

typedef struct {
    // Signals
    uint16_t EstimateRange; // [KM]
    uint8_t ChargeFETOn : 1;
    uint8_t DischargeFETOff : 1;
    uint8_t Charging : 1;
    uint8_t Discharging : 1;
    uint8_t FullyCharged : 1;
    uint8_t FullyDischarged : 1;
    uint8_t NearlyDischarged : 1;
    uint8_t ChargerDetected : 1;
    uint8_t Reserved : 8; // Battery Status 1
    uint8_t BatteryConfiguration : 2; // None
    uint8_t Reserved2 : 6;
    uint8_t PreviousError0; // None
    uint8_t PreviousError1; // None
    uint8_t Reserved3;
} BAT1_INFO00_ACK_T;

typedef struct {
    // Signals
} BAT1_INFO01_REQ_T;

typedef struct {
    // Signals
    uint8_t ActualVoltage[3]; // [mV]
    uint8_t Reserved;
    uint16_t ActualCurrent; // [mA]
    uint8_t RelativeStateOfCharge; // [%]
    uint8_t RelativeStateOfHealthy; // [%]
} BAT1_INFO01_ACK_T;

typedef struct {
    // Signals
} BAT1_INFO02_REQ_T;

typedef struct {
    // Signals
    uint8_t AbsoluteStateOfCharge[3]; // [mWh]
    uint8_t AbsoluteStateOfHealthy[3]; // [mWh]
    uint16_t DesignCapacity; // [Wh]
} BAT1_INFO02_ACK_T;

typedef struct {
    // Signals
} BAT1_INFO03_REQ_T;

typedef struct {
    // Signals
    uint32_t ProductionDate; // [UTC]
    uint16_t ChargeCycleCount; // [Count]
    uint16_t LongestDaysNoCharge : 11; // [Day]
    uint16_t ChargingStatus : 1; // None
    uint16_t reserved : 4;
} BAT1_INFO03_ACK_T;

typedef struct {
    // Signals
} BAT1_INFO04_REQ_T;

typedef struct {
    // Signals
    uint8_t AbsoluteStateOfCharge[3]; // [mWh]
    uint8_t Reserved;
    uint16_t MaxChargingCurrent; // [mA]
    uint16_t OverCurrentProtection; // [mA]
} BAT1_INFO04_ACK_T;

typedef struct {
    // Signals
} BAT1_INFO05_REQ_T;

typedef struct {
    // Signals
    uint8_t MaxVoltage[3]; // [mV]
    uint8_t Reserved;
    uint8_t OverVoltageProtection[3]; // [mV]
    uint8_t Reserved2;
} BAT1_INFO05_ACK_T;

typedef struct {
    // Signals
} BAT1_INFO06_REQ_T;

typedef struct {
    // Signals
    uint8_t TemperatureSensor1; // [C]
    uint8_t TemperatureSensor2; // [C]
    uint8_t TemperatureSensor3; // [C]
    uint8_t TemperatureSensor4; // [C]
    uint8_t OverTemperatureProtection; // [C]
    uint8_t UnderTemperatureProtection; // [C]
    uint8_t Reserved[2];
} BAT1_INFO06_ACK_T;

typedef struct {
    // Signals
} BAT1_INFO07_REQ_T;

typedef struct {
    // Signals
    uint32_t UTC; // [UTC]
    uint8_t Reserved[4];
} BAT1_INFO07_ACK_T;

typedef struct {
    // Signals
    uint8_t FromWho : 2; // None
    uint8_t SequenceNumber : 2; // None
    uint8_t MCUOperationCode : 4; // None
} BAT1_MCUOPERATION_REQ_T;

typedef struct {
    // Signals
    uint8_t FromWho : 2; // None
    uint8_t SequenceNumber : 2; // None
    uint8_t reserved:4; // Reserved
    uint8_t MCUResponseCode; // None
} BAT1_MCUOPERATION_ACK_T;

typedef struct {
    // Signals
    uint8_t FromWho : 2; // None
    uint8_t SequenceNumber : 2; // None
    uint8_t reserved : 4;
    uint8_t TestMode; // None
    uint8_t reserved1;
    uint8_t reserved2;
    uint32_t ValueofSpecificMode; // None
} BAT1_TESTMODE_REQ_T;

typedef struct {
    // Signals
    uint8_t FromWho : 2; // None
    uint8_t SequenceNumber : 2; // None
    uint8_t ResponseCode : 4; // None
} BAT1_TESTMODE_ACK_T;


/* External variable declarations */
extern BAT1_BASICCOMMAND0_REQ_T Bat1_BasicCommand0_REQ;
extern BAT1_BASICCOMMAND0_BRO_T Bat1_BasicCommand0_BRO;
extern BAT1_BASICCOMMAND1_REQ_T Bat1_BasicCommand1_REQ;
extern BAT1_BASICCOMMAND1_BRO_T Bat1_BasicCommand1_BRO;
extern BAT1_BASICINFO00_REQ_T Bat1_BasicInfo00_REQ;
extern BAT1_BASICINFO00_ACK_T Bat1_BasicInfo00;
extern BAT1_BASICINFO01_REQ_T Bat1_BasicInfo01_REQ;
extern BAT1_BASICINFO01_ACK_T Bat1_BasicInfo01;
extern BAT1_BASICINFO02_REQ_T Bat1_BasicInfo02_REQ;
extern BAT1_BASICINFO02_ACK_T Bat1_BasicInfo02;
extern BAT1_BASICINFO03_REQ_T Bat1_BasicInfo03_REQ;
extern BAT1_BASICINFO03_ACK_T Bat1_BasicInfo03;
extern BAT1_DEBUGMODE_REQ_T Bat1_DebugMode_REQ;
extern BAT1_DEBUGMODE_ACK_T Bat1_DebugMode;
extern BAT1_ERROR_REQ_T Bat1_Error_REQ;
extern BAT1_ERROR_ACK_T Bat1_Error;
extern BAT1_WARNING_BRO_T Bat1_Warning_BRO;
extern BAT1_INFO00_REQ_T Bat1_Info00_REQ;
extern BAT1_INFO00_ACK_T Bat1_Info00;
extern BAT1_INFO01_REQ_T Bat1_Info01_REQ;
extern BAT1_INFO01_ACK_T Bat1_Info01;
extern BAT1_INFO02_REQ_T Bat1_Info02_REQ;
extern BAT1_INFO02_ACK_T Bat1_Info02;
extern BAT1_INFO03_REQ_T Bat1_Info03_REQ;
extern BAT1_INFO03_ACK_T Bat1_Info03;
extern BAT1_INFO04_REQ_T Bat1_Info04_REQ;
extern BAT1_INFO04_ACK_T Bat1_Info04;
extern BAT1_INFO05_REQ_T Bat1_Info05_REQ;
extern BAT1_INFO05_ACK_T Bat1_Info05;
extern BAT1_INFO06_REQ_T Bat1_Info06_REQ;
extern BAT1_INFO06_ACK_T Bat1_Info06;
extern BAT1_INFO07_REQ_T Bat1_Info07_REQ;
extern BAT1_INFO07_ACK_T Bat1_Info07;
extern BAT1_MCUOPERATION_REQ_T Bat1_MCUOperation_REQ;
extern BAT1_MCUOPERATION_ACK_T Bat1_MCUOperation;
extern BAT1_TESTMODE_REQ_T Bat1_TestMode_REQ;
extern BAT1_TESTMODE_ACK_T Bat1_TestMode;

/* Function declarations */
void bat1_processQueueTX_ACK(S_CAN_QUEUE_INSTANCE* pQueue,uint32_t CAN_ID);
uint32_t bat1_processQueueRX_REQ(CanFrame frame);
void bat1_processQueueTX_BRO(S_CAN_QUEUE_INSTANCE* pQueue,uint32_t current_time);

#endif
