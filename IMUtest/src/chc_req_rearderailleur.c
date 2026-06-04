#include "chc_req_rearderailleur.h"

REARDERAILLEUR_BASICCOMMAND0_REQ_T RearDerailleur_BasicCommand0_REQ;
REARDERAILLEUR_BASICINFO00_REQ_T RearDerailleur_BasicInfo00_REQ;
REARDERAILLEUR_BASICINFO00_ACK_T RearDerailleur_BasicInfo00;
REARDERAILLEUR_BASICINFO01_REQ_T RearDerailleur_BasicInfo01_REQ;
REARDERAILLEUR_BASICINFO01_ACK_T RearDerailleur_BasicInfo01;
REARDERAILLEUR_BASICINFO02_REQ_T RearDerailleur_BasicInfo02_REQ;
REARDERAILLEUR_BASICINFO02_ACK_T RearDerailleur_BasicInfo02;
REARDERAILLEUR_BASICINFO03_REQ_T RearDerailleur_BasicInfo03_REQ;
REARDERAILLEUR_BASICINFO03_ACK_T RearDerailleur_BasicInfo03;
REARDERAILLEUR_DEBUGMODE_REQ_T RearDerailleur_DebugMode_REQ;
REARDERAILLEUR_DEBUGMODE_ACK_T RearDerailleur_DebugMode;
REARDERAILLEUR_ERROR_REQ_T RearDerailleur_Error_REQ;
REARDERAILLEUR_ERROR_ACK_T RearDerailleur_Error;
REARDERAILLEUR_INFO00_REQ_T RearDerailleur_Info00_REQ;
REARDERAILLEUR_INFO00_ACK_T RearDerailleur_Info00;
REARDERAILLEUR_MCUOPERATION_REQ_T RearDerailleur_MCUOperation_REQ;
REARDERAILLEUR_MCUOPERATION_ACK_T RearDerailleur_MCUOperation;
REARDERAILLEUR_TESTMODE_REQ_T RearDerailleur_TestMode_REQ;
REARDERAILLEUR_TESTMODE_ACK_T RearDerailleur_TestMode;

// 接收 ACK / BRO 訊號
uint32_t rearDerailleur_processQueueRX_ACKBRO(CanFrame frame) {
    switch (frame.id) {
        case REARDERAILLEUR_BASICINFO03ACK:
        case REARDERAILLEUR_BASICINFO03BRO:
            memcpy(&RearDerailleur_BasicInfo03, frame.data, sizeof(RearDerailleur_BasicInfo03));
            return frame.id;
            break;

        case REARDERAILLEUR_BASICINFO02ACK:
        case REARDERAILLEUR_BASICINFO02BRO:
            memcpy(&RearDerailleur_BasicInfo02, frame.data, sizeof(RearDerailleur_BasicInfo02));
            return frame.id;
            break;

        case REARDERAILLEUR_BASICINFO01ACK:
        case REARDERAILLEUR_BASICINFO01BRO:
            memcpy(&RearDerailleur_BasicInfo01, frame.data, sizeof(RearDerailleur_BasicInfo01));
            return frame.id;
            break;

        case REARDERAILLEUR_BASICINFO00ACK:
        case REARDERAILLEUR_BASICINFO00BRO:
            memcpy(&RearDerailleur_BasicInfo00, frame.data, sizeof(RearDerailleur_BasicInfo00));
            return frame.id;
            break;

        case REARDERAILLEUR_MCUOPERATIONACK:
            memcpy(&RearDerailleur_MCUOperation, frame.data, sizeof(RearDerailleur_MCUOperation));
            return frame.id;
            break;

        case REARDERAILLEUR_DEBUGMODEACK:
            memcpy(&RearDerailleur_DebugMode, frame.data, sizeof(RearDerailleur_DebugMode));
            return frame.id;
            break;

        case REARDERAILLEUR_ERRORACK:
        case REARDERAILLEUR_ERRORBRO:
            memcpy(&RearDerailleur_Error, frame.data, sizeof(RearDerailleur_Error));
            return frame.id;
            break;

        case REARDERAILLEUR_INFO00ACK:
        case REARDERAILLEUR_INFO00BRO:
            memcpy(&RearDerailleur_Info00, frame.data, sizeof(RearDerailleur_Info00));
            return frame.id;
            break;

        case REARDERAILLEUR_TESTMODEACK:
            memcpy(&RearDerailleur_TestMode, frame.data, sizeof(RearDerailleur_TestMode));
            return frame.id;
            break;

        default:
            return 0; // 返回 0 表示已處理 ACK
    }
}
void rearDerailleur_processQueueTX_REQ(uint32_t CANID) {
    CanFrame frame;
    frame.id = CANID;
    switch (CANID) {
        case REARDERAILLEUR_BASICCOMMAND0REQ:
            frame.dlc = sizeof(RearDerailleur_BasicCommand0_REQ);
            memcpy(frame.data, &RearDerailleur_BasicCommand0_REQ, frame.dlc);
            break;

        case REARDERAILLEUR_BASICINFO00REQ:
            frame.dlc = sizeof(RearDerailleur_BasicInfo00_REQ);
            memcpy(frame.data, &RearDerailleur_BasicInfo00_REQ, frame.dlc);
            break;

        case REARDERAILLEUR_BASICINFO01REQ:
            frame.dlc = sizeof(RearDerailleur_BasicInfo01_REQ);
            memcpy(frame.data, &RearDerailleur_BasicInfo01_REQ, frame.dlc);
            break;

        case REARDERAILLEUR_BASICINFO02REQ:
            frame.dlc = sizeof(RearDerailleur_BasicInfo02_REQ);
            memcpy(frame.data, &RearDerailleur_BasicInfo02_REQ, frame.dlc);
            break;

        case REARDERAILLEUR_BASICINFO03REQ:
            frame.dlc = sizeof(RearDerailleur_BasicInfo03_REQ);
            memcpy(frame.data, &RearDerailleur_BasicInfo03_REQ, frame.dlc);
            break;

        case REARDERAILLEUR_DEBUGMODEREQ:
            frame.dlc = sizeof(RearDerailleur_DebugMode_REQ);
            memcpy(frame.data, &RearDerailleur_DebugMode_REQ, frame.dlc);
            break;

        case REARDERAILLEUR_ERRORREQ:
            frame.dlc = sizeof(RearDerailleur_Error_REQ);
            memcpy(frame.data, &RearDerailleur_Error_REQ, frame.dlc);
            break;

        case REARDERAILLEUR_INFO00REQ:
            frame.dlc = sizeof(RearDerailleur_Info00_REQ);
            memcpy(frame.data, &RearDerailleur_Info00_REQ, frame.dlc);
            break;

        case REARDERAILLEUR_MCUOPERATIONREQ:
            frame.dlc = sizeof(RearDerailleur_MCUOperation_REQ);
            memcpy(frame.data, &RearDerailleur_MCUOperation_REQ, frame.dlc);
            break;

        case REARDERAILLEUR_TESTMODEREQ:
            frame.dlc = sizeof(RearDerailleur_TestMode_REQ);
            memcpy(frame.data, &RearDerailleur_TestMode_REQ, frame.dlc);
            break;

        default:
            return;
    }
    enqueueTXQueue(&frame);
}
