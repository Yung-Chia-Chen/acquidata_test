#ifndef _CHC_REQ_CONTROLLER_H_
#define _CHC_REQ_CONTROLLER_H_

#include <stdint.h>
#include <string.h>
#include "chc_can_queue.h"

/* CAN message IDs */
#define CONTROLLER_BASICINFO03REQ 0x1E942031U
#define CONTROLLER_BASICINFO03BRO 0x1E942032U
#define CONTROLLER_BASICINFO03ACK 0x1E942030U // ACK ID
#define CONTROLLER_BASICINFO02REQ 0x1E94202DU
#define CONTROLLER_BASICINFO02BRO 0x1E94202EU
#define CONTROLLER_BASICINFO02ACK 0x1E94202CU // ACK ID
#define CONTROLLER_BASICINFO01REQ 0x1E942029U
#define CONTROLLER_BASICINFO01BRO 0x1E94202AU
#define CONTROLLER_BASICINFO01ACK 0x1E942028U // ACK ID
#define CONTROLLER_BASICINFO00REQ 0x1E942025U
#define CONTROLLER_BASICINFO00BRO 0x1E942026U
#define CONTROLLER_BASICINFO00ACK 0x1E942024U // ACK ID
#define CONTROLLER_MCUOPERATIONACK 0x1E94200DU // ACK ID
#define CONTROLLER_MCUOPERATIONREQ 0x1E94200CU
#define CONTROLLER_ERRORREQ 0x1E9420BDU
#define CONTROLLER_ERRORBRO 0x1E9420BEU
#define CONTROLLER_ERRORACK 0x1E9420BCU // ACK ID
#define CONTROLLER_INFO03REQ 0x1E94204DU
#define CONTROLLER_INFO03BRO 0x1E94204EU
#define CONTROLLER_INFO03ACK 0x1E94204CU // ACK ID
#define CONTROLLER_INFO02REQ 0x1E942049U
#define CONTROLLER_INFO02BRO 0x1E94204AU
#define CONTROLLER_INFO02ACK 0x1E942048U // ACK ID
#define CONTROLLER_INFO01REQ 0x1E942045U
#define CONTROLLER_INFO01BRO 0x1E942046U
#define CONTROLLER_INFO01ACK 0x1E942044U // ACK ID
#define CONTROLLER_INFO00REQ 0x1E942041U
#define CONTROLLER_INFO00BRO 0x1E942042U
#define CONTROLLER_INFO00ACK 0x1E942040U // ACK ID
#define CONTROLLER_DEBUGMODEACK 0x1E942080U // ACK ID
#define CONTROLLER_DEBUGMODEREQ 0x1E942081U
#define CONTROLLER_TESTMODEREQ 0x1E94203DU
#define CONTROLLER_TESTMODEACK 0x1E94203CU // ACK ID

/* Structure definitions */
typedef struct {
    // Signals
    uint16_t BroadcastPeriodChange; // None
} CONTROLLER_BASICINFO00_REQ_T;

typedef struct {
    // Signals
    uint8_t MID[6]; // None
    uint8_t BikeModelVersionASCII; // None
    uint8_t BikeModelVersionBCD; // None
} CONTROLLER_BASICINFO00_ACK_T;

typedef struct {
    // Signals
    uint16_t BroadcastPeriodChange; // None
} CONTROLLER_BASICINFO01_REQ_T;

typedef struct {
    // Signals
    uint8_t FirmwareVersion_ASCII; // None
    uint8_t FirmwareVersion_BCD; // None
    uint8_t HWVersionASCII; // None
    uint8_t HWVersionBCD; // None
    uint8_t KernelVersionASCII; // None
    uint8_t KernelVersionBCD; // None
    uint8_t BootloaderVersionASCII; // None
    uint8_t BootloaderVersionBCD; // None
} CONTROLLER_BASICINFO01_ACK_T;

typedef struct {
    // Signals
    uint16_t BroadcastPeriodChange; // None
} CONTROLLER_BASICINFO02_REQ_T;

typedef struct {
    // Signals
    uint16_t LockConfiguration; // None
    uint8_t RegistrationStatus; // None
    uint8_t ApplicationFlag; // None

    uint8_t SupportProtocolType; // None
    uint16_t SupportProtocolVersion; // None
} CONTROLLER_BASICINFO02_ACK_T;

typedef struct {
    // Signals
    uint16_t BroadcastPeriodChange; // None
} CONTROLLER_BASICINFO03_REQ_T;

typedef struct {
    // Signals
    uint8_t BikeID[6]; // None
} CONTROLLER_BASICINFO03_ACK_T;

typedef struct {
    // Signals
    uint8_t FromWho : 2; // None
    uint8_t SequenceNumber : 2; // None
    uint8_t reserved:4;
    uint8_t DebugInfoNumber; // None
    uint16_t TimeInterval; // [ms]
    uint8_t Counter; // None
} CONTROLLER_DEBUGMODE_REQ_T;

typedef struct {
    // Signals
    uint8_t FromWho : 2; // None
    uint8_t SequenceNumber : 2; // None
    uint8_t ResponseCode : 4; // None
} CONTROLLER_DEBUGMODE_ACK_T;

typedef struct {
    // Signals
} CONTROLLER_ERROR_REQ_T;

typedef struct {
    // Signals
    uint8_t Controller_ER0_0 : 1; // None
    uint8_t Controller_ER0_1 : 1; // None
    uint8_t Controller_ER0_2 : 1; // None
    uint8_t Controller_ER0_3 : 1; // None
    uint8_t Controller_ER0_4 : 1; // None
    uint8_t Controller_ER0_5 : 1; // None
    uint8_t Controller_ER0_6 : 1; // None
    uint8_t Controller_ER0_7 : 1; // None
    uint8_t Controller_ER1_0 : 1; // None
    uint8_t Controller_ER1_1 : 1; // None
    uint8_t Controller_ER1_2 : 1; // None
    uint8_t Controller_ER1_3 : 1; // None
    uint8_t Controller_ER1_4 : 1; // None
    uint8_t Controller_ER1_5 : 1; // None
    uint8_t Controller_ER1_6 : 1; // None
    uint8_t Controller_ER1_7 : 1; // None
    uint8_t Controller_ER2_0 : 1; // None
    uint8_t Controller_ER2_1 : 1; // None
    uint8_t Controller_ER2_2 : 1; // None
    uint8_t Controller_ER2_3 : 1; // None
    uint8_t Controller_ER2_4 : 1; // None
    uint8_t Controller_ER2_5 : 1; // None
    uint8_t Controller_ER2_6 : 1; // None
    uint8_t Controller_ER2_7 : 1; // None
    uint8_t Controller_ER3_0 : 1; // None
    uint8_t Controller_ER3_1 : 1; // None
    uint8_t Controller_ER3_2 : 1; // None
    uint8_t Controller_ER3_3 : 1; // None
    uint8_t Controller_ER3_4 : 1; // None
    uint8_t Controller_ER3_5 : 1; // None
    uint8_t Controller_ER3_6 : 1; // None
    uint8_t Controller_ER3_7 : 1; // None
} CONTROLLER_ERROR_ACK_T;

typedef struct {
    // Signals
} CONTROLLER_INFO00_REQ_T;

typedef struct {
    // Signals
    uint16_t BikeSpeed; // [KPH]
    uint16_t LimitSpeed; // [KPH]
    uint8_t LightStatus : 1; // None
    uint8_t LightControllable : 1; // None
    uint32_t OdoMeter : 30; // [meter]
} CONTROLLER_INFO00_ACK_T;

typedef struct {
    // Signals
} CONTROLLER_INFO01_REQ_T;

typedef struct {
    // Signals
    uint16_t ActualCurrent; // [mA]
    uint16_t AverageCurrent; // [mA]
    uint16_t TargetCurrent; // [mA]
    uint16_t LimitCurrent; // [mA]
} CONTROLLER_INFO01_ACK_T;

typedef struct {
    // Signals
} CONTROLLER_INFO02_REQ_T;

typedef struct {
    // Signals
    uint16_t WheelRPMBySpeedSensor; // [RPM]
    uint16_t WheelSpeedByHallSensor; // [RPM]
    uint16_t MotorRPM; // [PRM]
    uint8_t MotorTemperature; // [C]
} CONTROLLER_INFO02_ACK_T;

typedef struct {
    // Signals
} CONTROLLER_INFO03_REQ_T;

typedef struct {
    // Signals
    uint32_t TreadlePower : 24; // [mW]
    uint32_t TreadleCadence:8; // [RPM]
    uint16_t TreadleTorque; // [Nm]
    uint8_t TreadleDirection : 1; // None
    uint8_t TreadleStatus : 1; // None
    uint8_t reserved:6;
    uint8_t ControllerTemperature; // [C]
} CONTROLLER_INFO03_ACK_T;

typedef struct {
    // Signals
    uint8_t FromWho : 2; // None
    uint8_t SequenceNumber : 2; // None
    uint8_t MCUOperationCode : 4; // None
} CONTROLLER_MCUOPERATION_REQ_T;

typedef struct {
    // Signals
    uint8_t FromWho : 2; // None
    uint8_t SequenceNumber : 2; // None
    uint8_t reserved : 4; // Reserved
    uint8_t MCUResponseCode; // None
} CONTROLLER_MCUOPERATION_ACK_T;

typedef struct {
    // Signals
    uint8_t FromWho : 2; // None
    uint8_t SequenceNumber : 2; // None
    uint8_t reserved:4;
    uint8_t TestMode; // None
    uint8_t reserved1;
    uint8_t reserved2;
    uint32_t ValueofSpecificMode; // None
} CONTROLLERTESTMODE_REQ_T;

typedef struct {
    // Signals
    uint8_t FromWho : 2; // None
    uint8_t SequenceNumber : 2; // None
    uint8_t ResponseCode : 4; // None
} CONTROLLERTESTMODE_ACK_T;


/* External variable declarations */
extern CONTROLLER_BASICINFO00_REQ_T Controller_BasicInfo00_REQ;
extern CONTROLLER_BASICINFO00_ACK_T Controller_BasicInfo00;
extern CONTROLLER_BASICINFO01_REQ_T Controller_BasicInfo01_REQ;
extern CONTROLLER_BASICINFO01_ACK_T Controller_BasicInfo01;
extern CONTROLLER_BASICINFO02_REQ_T Controller_BasicInfo02_REQ;
extern CONTROLLER_BASICINFO02_ACK_T Controller_BasicInfo02;
extern CONTROLLER_BASICINFO03_REQ_T Controller_BasicInfo03_REQ;
extern CONTROLLER_BASICINFO03_ACK_T Controller_BasicInfo03;
extern CONTROLLER_DEBUGMODE_REQ_T Controller_DebugMode_REQ;
extern CONTROLLER_DEBUGMODE_ACK_T Controller_DebugMode;
extern CONTROLLER_ERROR_REQ_T Controller_Error_REQ;
extern CONTROLLER_ERROR_ACK_T Controller_Error;
extern CONTROLLER_INFO00_REQ_T Controller_Info00_REQ;
extern CONTROLLER_INFO00_ACK_T Controller_Info00;
extern CONTROLLER_INFO01_REQ_T Controller_Info01_REQ;
extern CONTROLLER_INFO01_ACK_T Controller_Info01;
extern CONTROLLER_INFO02_REQ_T Controller_Info02_REQ;
extern CONTROLLER_INFO02_ACK_T Controller_Info02;
extern CONTROLLER_INFO03_REQ_T Controller_Info03_REQ;
extern CONTROLLER_INFO03_ACK_T Controller_Info03;
extern CONTROLLER_MCUOPERATION_REQ_T Controller_MCUOperation_REQ;
extern CONTROLLER_MCUOPERATION_ACK_T Controller_MCUOperation;
extern CONTROLLERTESTMODE_REQ_T Controller_TestMode_REQ;
extern CONTROLLERTESTMODE_ACK_T Controller_TestMode;

/* Function declarations */
uint32_t controller_processQueueRX_ACKBRO(CanFrame frame);
void controller_processQueueTX_REQ(uint32_t CANID);
#endif
