#include "chc_req_controller.h"

CONTROLLER_BASICINFO00_REQ_T Controller_BasicInfo00_REQ;
CONTROLLER_BASICINFO00_ACK_T Controller_BasicInfo00;
CONTROLLER_BASICINFO01_REQ_T Controller_BasicInfo01_REQ;
CONTROLLER_BASICINFO01_ACK_T Controller_BasicInfo01;
CONTROLLER_BASICINFO02_REQ_T Controller_BasicInfo02_REQ;
CONTROLLER_BASICINFO02_ACK_T Controller_BasicInfo02;
CONTROLLER_BASICINFO03_REQ_T Controller_BasicInfo03_REQ;
CONTROLLER_BASICINFO03_ACK_T Controller_BasicInfo03;
CONTROLLER_DEBUGMODE_REQ_T Controller_DebugMode_REQ;
CONTROLLER_DEBUGMODE_ACK_T Controller_DebugMode;
CONTROLLER_ERROR_REQ_T Controller_Error_REQ;
CONTROLLER_ERROR_ACK_T Controller_Error;
CONTROLLER_INFO00_REQ_T Controller_Info00_REQ;
CONTROLLER_INFO00_ACK_T Controller_Info00;
CONTROLLER_INFO01_REQ_T Controller_Info01_REQ;
CONTROLLER_INFO01_ACK_T Controller_Info01;
CONTROLLER_INFO02_REQ_T Controller_Info02_REQ;
CONTROLLER_INFO02_ACK_T Controller_Info02;
CONTROLLER_INFO03_REQ_T Controller_Info03_REQ;
CONTROLLER_INFO03_ACK_T Controller_Info03;
CONTROLLER_MCUOPERATION_REQ_T Controller_MCUOperation_REQ;
CONTROLLER_MCUOPERATION_ACK_T Controller_MCUOperation;
CONTROLLERTESTMODE_REQ_T Controller_TestMode_REQ;
CONTROLLERTESTMODE_ACK_T Controller_TestMode;

// 接收 ACK / BRO 訊號
uint32_t controller_processQueueRX_ACKBRO(CanFrame frame) {
    switch (frame.id) {
        case CONTROLLER_BASICINFO03ACK:
        case CONTROLLER_BASICINFO03BRO:
            memcpy(&Controller_BasicInfo03, frame.data, sizeof(Controller_BasicInfo03));
            return frame.id;
            break;

        case CONTROLLER_BASICINFO02ACK:
        case CONTROLLER_BASICINFO02BRO:
            memcpy(&Controller_BasicInfo02, frame.data, sizeof(Controller_BasicInfo02));
            return frame.id;
            break;

        case CONTROLLER_BASICINFO01ACK:
        case CONTROLLER_BASICINFO01BRO:
            memcpy(&Controller_BasicInfo01, frame.data, sizeof(Controller_BasicInfo01));
            return frame.id;
            break;

        case CONTROLLER_BASICINFO00ACK:
        case CONTROLLER_BASICINFO00BRO:
            memcpy(&Controller_BasicInfo00, frame.data, sizeof(Controller_BasicInfo00));
            return frame.id;
            break;

        case CONTROLLER_MCUOPERATIONACK:
            memcpy(&Controller_MCUOperation, frame.data, sizeof(Controller_MCUOperation));
            return frame.id;
            break;

        case CONTROLLER_ERRORACK:
        case CONTROLLER_ERRORBRO:
            memcpy(&Controller_Error, frame.data, sizeof(Controller_Error));
            return frame.id;
            break;

        case CONTROLLER_INFO03ACK:
        case CONTROLLER_INFO03BRO:
            memcpy(&Controller_Info03, frame.data, sizeof(Controller_Info03));
            return frame.id;
            break;

        case CONTROLLER_INFO02ACK:
        case CONTROLLER_INFO02BRO:
            memcpy(&Controller_Info02, frame.data, sizeof(Controller_Info02));
            return frame.id;
            break;

        case CONTROLLER_INFO01ACK:
        case CONTROLLER_INFO01BRO:
            memcpy(&Controller_Info01, frame.data, sizeof(Controller_Info01));
            return frame.id;
            break;

        case CONTROLLER_INFO00ACK:
        case CONTROLLER_INFO00BRO:
            memcpy(&Controller_Info00, frame.data, sizeof(Controller_Info00));
            return frame.id;
            break;

        case CONTROLLER_DEBUGMODEACK:
            memcpy(&Controller_DebugMode, frame.data, sizeof(Controller_DebugMode));
            return frame.id;
            break;

        case CONTROLLER_TESTMODEACK:
            memcpy(&Controller_TestMode, frame.data, sizeof(Controller_TestMode));
            return frame.id;
            break;

        default:
            return 0; // 返回 0 表示已處理 ACK
    }
}
// 發送請求

void controller_processQueueTX_REQ(uint32_t CANID) {
    CanFrame frame;
    frame.id = CANID;

    switch (CANID) {
        case CONTROLLER_BASICINFO00REQ:
            frame.dlc = sizeof(Controller_BasicInfo00_REQ);
            memcpy(frame.data, &Controller_BasicInfo00_REQ, frame.dlc);
            break;

        case CONTROLLER_BASICINFO01REQ:
            frame.dlc = sizeof(Controller_BasicInfo01_REQ);
            memcpy(frame.data, &Controller_BasicInfo01_REQ, frame.dlc);
            break;

        case CONTROLLER_BASICINFO02REQ:
            frame.dlc = sizeof(Controller_BasicInfo02_REQ);
            memcpy(frame.data, &Controller_BasicInfo02_REQ, frame.dlc);
            break;

        case CONTROLLER_BASICINFO03REQ:
            frame.dlc = sizeof(Controller_BasicInfo03_REQ);
            memcpy(frame.data, &Controller_BasicInfo03_REQ, frame.dlc);
            break;

        case CONTROLLER_DEBUGMODEREQ:
            frame.dlc = sizeof(Controller_DebugMode_REQ);
            memcpy(frame.data, &Controller_DebugMode_REQ, frame.dlc);
            break;

        case CONTROLLER_ERRORREQ:
            frame.dlc = sizeof(Controller_Error_REQ);
            memcpy(frame.data, &Controller_Error_REQ, frame.dlc);
            break;

        case CONTROLLER_INFO00REQ:
            frame.dlc = sizeof(Controller_Info00_REQ);
            memcpy(frame.data, &Controller_Info00_REQ, frame.dlc);
            break;

        case CONTROLLER_INFO01REQ:
            frame.dlc = sizeof(Controller_Info01_REQ);
            memcpy(frame.data, &Controller_Info01_REQ, frame.dlc);
            break;

        case CONTROLLER_INFO02REQ:
            frame.dlc = sizeof(Controller_Info02_REQ);
            memcpy(frame.data, &Controller_Info02_REQ, frame.dlc);
            break;

        case CONTROLLER_INFO03REQ:
            frame.dlc = sizeof(Controller_Info03_REQ);
            memcpy(frame.data, &Controller_Info03_REQ, frame.dlc);
            break;

        case CONTROLLER_MCUOPERATIONREQ:
            frame.dlc = sizeof(Controller_MCUOperation_REQ);
            memcpy(frame.data, &Controller_MCUOperation_REQ, frame.dlc);
            break;

        case CONTROLLER_TESTMODEREQ:
            frame.dlc = sizeof(Controller_TestMode_REQ);
            memcpy(frame.data, &Controller_TestMode_REQ, frame.dlc);
            break;

        default:
            // ?��??��? CANID，�???
            return;
    }

    // ?�送數??
    enqueueTXQueue(&frame); // 發送數據
}
