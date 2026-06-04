#ifndef _CHC_REQ_REARDERAILLEUR_H_
#define _CHC_REQ_REARDERAILLEUR_H_

#include <stdint.h>
#include <string.h>
#include "chc_can_queue.h"

/* CAN message IDs */
#define REARDERAILLEUR_BASICINFO03REQ 0x1E944831U
#define REARDERAILLEUR_BASICINFO03BRO 0x1E944832U
#define REARDERAILLEUR_BASICINFO03ACK 0x1E944830U // ACK ID
#define REARDERAILLEUR_BASICINFO02REQ 0x1E94482DU
#define REARDERAILLEUR_BASICINFO02BRO 0x1E94482EU
#define REARDERAILLEUR_BASICINFO02ACK 0x1E94482CU // ACK ID
#define REARDERAILLEUR_BASICINFO01REQ 0x1E944829U
#define REARDERAILLEUR_BASICINFO01BRO 0x1E94482AU
#define REARDERAILLEUR_BASICINFO01ACK 0x1E944828U // ACK ID
#define REARDERAILLEUR_BASICINFO00REQ 0x1E944825U
#define REARDERAILLEUR_BASICINFO00BRO 0x1E944826U
#define REARDERAILLEUR_BASICINFO00ACK 0x1E944824U // ACK ID
#define REARDERAILLEUR_MCUOPERATIONACK 0x1E94480DU // ACK ID
#define REARDERAILLEUR_MCUOPERATIONREQ 0x1E94480CU
#define REARDERAILLEUR_DEBUGMODEACK 0x1E944880U // ACK ID
#define REARDERAILLEUR_DEBUGMODEREQ 0x1E944881U
#define REARDERAILLEUR_ERRORREQ 0x1E9448BDU
#define REARDERAILLEUR_ERRORBRO 0x1E9448BEU
#define REARDERAILLEUR_ERRORACK 0x1E9448BCU // ACK ID
#define REARDERAILLEUR_INFO00REQ 0x1E944841U
#define REARDERAILLEUR_INFO00BRO 0x1E944842U
#define REARDERAILLEUR_INFO00ACK 0x1E944840U // ACK ID
#define REARDERAILLEUR_TESTMODEACK 0x1E94483CU // ACK ID
#define REARDERAILLEUR_TESTMODEREQ 0x1E94483DU
#define REARDERAILLEUR_BASICCOMMAND0REQ 0x1E944835U

/* Structure definitions */
typedef struct {
    // Signals
    uint8_t OperationCode; // None
} REARDERAILLEUR_BASICCOMMAND0_REQ_T;

typedef struct {
    // Signals
    uint16_t BroadcastPeriodChange; // None
    uint8_t reserved[6];
} REARDERAILLEUR_BASICINFO00_REQ_T;

typedef struct {
    // Signals
    uint8_t MID[6]; // None
    uint8_t BikeModelVersionASCII; // None
    uint8_t BikeModelVersionBCD; // None
} REARDERAILLEUR_BASICINFO00_ACK_T;

typedef struct {
    // Signals
    uint16_t BroadcastPeriodChange; // None
    uint8_t reserved[6];
} REARDERAILLEUR_BASICINFO01_REQ_T;

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
} REARDERAILLEUR_BASICINFO01_ACK_T;

typedef struct {
    // Signals
    uint16_t BroadcastPeriodChange; // None
    uint8_t reserved[6];
} REARDERAILLEUR_BASICINFO02_REQ_T;

typedef struct {
    // Signals
    uint16_t LockConfiguration; // None
    uint8_t RegistrationStatus; // None
    uint8_t ApplicationFlag; // None
    uint8_t SupportProtocolType; // None
    uint16_t SupportProtocolVersion; // None
    uint8_t reserved; // Protocol byte 7: Reserved
} REARDERAILLEUR_BASICINFO02_ACK_T;

typedef struct {
    // Signals
    uint16_t BroadcastPeriodChange; // None
    uint8_t reserved[6];
} REARDERAILLEUR_BASICINFO03_REQ_T;

typedef struct {
    // Signals
    uint8_t BikeID[6]; // None
    uint8_t reserved[2]; // Protocol bytes 6-7: Reserved
} REARDERAILLEUR_BASICINFO03_ACK_T;

typedef struct {
    // Signals
    uint8_t FromWho : 2; // None
    uint8_t SequenceNumber : 2; // None
    uint8_t reserved:4;
    uint8_t DebugInfoNumber; // None
    uint16_t TimeInterval; // [ms]
    uint8_t Counter; // None
} REARDERAILLEUR_DEBUGMODE_REQ_T;

typedef struct {
    // Signals
    uint8_t FromWho : 2; // None
    uint8_t SequenceNumber : 2; // None
    uint8_t ResponseCode : 4; // None
} REARDERAILLEUR_DEBUGMODE_ACK_T;

typedef struct {
    // Signals
} REARDERAILLEUR_ERROR_REQ_T;

typedef struct {
    // Signals
    uint8_t ER0; // None
} REARDERAILLEUR_ERROR_ACK_T;

typedef struct {
    // Signals
} REARDERAILLEUR_INFO00_REQ_T;

typedef struct {
    // Signals
    uint8_t GearIndex; // None
    uint8_t GearRange; // None
    uint8_t FirmwareVersion_ASCII; // None
    uint8_t FirmwareVersion_BCD; // None
    uint8_t GearOffsetNumber; // None
} REARDERAILLEUR_INFO00_ACK_T;

typedef struct {
    // Signals
    uint8_t FromWho : 2; // None
    uint8_t SequenceNumber : 2; // None
    uint8_t MCUOperationCode : 4; // None
} REARDERAILLEUR_MCUOPERATION_REQ_T;

typedef struct {
    // Signals
    uint8_t FromWho : 2; // None
    uint8_t SequenceNumber : 2; // None
    uint8_t reserved:4;
    uint8_t MCUResponseCode; // None
} REARDERAILLEUR_MCUOPERATION_ACK_T;

typedef struct {
    // Signals
    uint8_t FromWho : 2; // None
    uint8_t SequenceNumber : 2; // None
    uint8_t reserved:4;
    uint8_t TestMode; // None
    uint8_t reserved1;
    uint8_t reserved2;
    uint32_t ValueofSpecificMode; // None
} REARDERAILLEUR_TESTMODE_REQ_T;

typedef struct {
    // Signals
    uint8_t FromWho : 2; // None
    uint8_t SequenceNumber : 2; // None
    uint8_t ResponseCode : 4; // None
} REARDERAILLEUR_TESTMODE_ACK_T;


/* External variable declarations */
extern REARDERAILLEUR_BASICCOMMAND0_REQ_T RearDerailleur_BasicCommand0_REQ;
extern REARDERAILLEUR_BASICINFO00_REQ_T RearDerailleur_BasicInfo00_REQ;
extern REARDERAILLEUR_BASICINFO00_ACK_T RearDerailleur_BasicInfo00;
extern REARDERAILLEUR_BASICINFO01_REQ_T RearDerailleur_BasicInfo01_REQ;
extern REARDERAILLEUR_BASICINFO01_ACK_T RearDerailleur_BasicInfo01;
extern REARDERAILLEUR_BASICINFO02_REQ_T RearDerailleur_BasicInfo02_REQ;
extern REARDERAILLEUR_BASICINFO02_ACK_T RearDerailleur_BasicInfo02;
extern REARDERAILLEUR_BASICINFO03_REQ_T RearDerailleur_BasicInfo03_REQ;
extern REARDERAILLEUR_BASICINFO03_ACK_T RearDerailleur_BasicInfo03;
extern REARDERAILLEUR_DEBUGMODE_REQ_T RearDerailleur_DebugMode_REQ;
extern REARDERAILLEUR_DEBUGMODE_ACK_T RearDerailleur_DebugMode;
extern REARDERAILLEUR_ERROR_REQ_T RearDerailleur_Error_REQ;
extern REARDERAILLEUR_ERROR_ACK_T RearDerailleur_Error;
extern REARDERAILLEUR_INFO00_REQ_T RearDerailleur_Info00_REQ;
extern REARDERAILLEUR_INFO00_ACK_T RearDerailleur_Info00;
extern REARDERAILLEUR_MCUOPERATION_REQ_T RearDerailleur_MCUOperation_REQ;
extern REARDERAILLEUR_MCUOPERATION_ACK_T RearDerailleur_MCUOperation;
extern REARDERAILLEUR_TESTMODE_REQ_T RearDerailleur_TestMode_REQ;
extern REARDERAILLEUR_TESTMODE_ACK_T RearDerailleur_TestMode;

/* Function declarations */
uint32_t rearDerailleur_processQueueRX_ACKBRO(CanFrame frame);
void rearDerailleur_processQueueTX_REQ(uint32_t CANID);
#endif
