#include "chc_req_bat1.h"

BAT1_BASICCOMMAND0_REQ_T Bat1_BasicCommand0_REQ;
BAT1_BASICCOMMAND0_BRO_T Bat1_BasicCommand0_BRO;
BAT1_BASICCOMMAND1_REQ_T Bat1_BasicCommand1_REQ;
BAT1_BASICCOMMAND1_BRO_T Bat1_BasicCommand1_BRO;
BAT1_BASICINFO00_REQ_T Bat1_BasicInfo00_REQ;
BAT1_BASICINFO00_ACK_T Bat1_BasicInfo00;
BAT1_BASICINFO01_REQ_T Bat1_BasicInfo01_REQ;
BAT1_BASICINFO01_ACK_T Bat1_BasicInfo01;
BAT1_BASICINFO02_REQ_T Bat1_BasicInfo02_REQ;
BAT1_BASICINFO02_ACK_T Bat1_BasicInfo02;
BAT1_BASICINFO03_REQ_T Bat1_BasicInfo03_REQ;
BAT1_BASICINFO03_ACK_T Bat1_BasicInfo03;
BAT1_DEBUGMODE_REQ_T Bat1_DebugMode_REQ;
BAT1_DEBUGMODE_ACK_T Bat1_DebugMode;
BAT1_ERROR_REQ_T Bat1_Error_REQ;
BAT1_ERROR_ACK_T Bat1_Error;
BAT1_WARNING_BRO_T Bat1_Warning_BRO;
BAT1_INFO00_REQ_T Bat1_Info00_REQ;
BAT1_INFO00_ACK_T Bat1_Info00;
BAT1_INFO01_REQ_T Bat1_Info01_REQ;
BAT1_INFO01_ACK_T Bat1_Info01;
BAT1_INFO02_REQ_T Bat1_Info02_REQ;
BAT1_INFO02_ACK_T Bat1_Info02;
BAT1_INFO03_REQ_T Bat1_Info03_REQ;
BAT1_INFO03_ACK_T Bat1_Info03;
BAT1_INFO04_REQ_T Bat1_Info04_REQ;
BAT1_INFO04_ACK_T Bat1_Info04;
BAT1_INFO05_REQ_T Bat1_Info05_REQ;
BAT1_INFO05_ACK_T Bat1_Info05;
BAT1_INFO06_REQ_T Bat1_Info06_REQ;
BAT1_INFO06_ACK_T Bat1_Info06;
BAT1_INFO07_REQ_T Bat1_Info07_REQ;
BAT1_INFO07_ACK_T Bat1_Info07;
BAT1_MCUOPERATION_REQ_T Bat1_MCUOperation_REQ;
BAT1_MCUOPERATION_ACK_T Bat1_MCUOperation;
BAT1_TESTMODE_REQ_T Bat1_TestMode_REQ;
BAT1_TESTMODE_ACK_T Bat1_TestMode;

// 接收 ACK / BRO 訊號
uint32_t bat1_processQueueRX_ACKBRO(CanFrame frame) {
    switch (frame.id) {
        case BAT1_BASICINFO03ACK:
        case BAT1_BASICINFO03BRO:
            memcpy(&Bat1_BasicInfo03, frame.data, sizeof(Bat1_BasicInfo03));
            return frame.id;
            break;

        case BAT1_BASICINFO02ACK:
        case BAT1_BASICINFO02BRO:
            memcpy(&Bat1_BasicInfo02, frame.data, sizeof(Bat1_BasicInfo02));
            return frame.id;
            break;

        case BAT1_BASICINFO01ACK:
        case BAT1_BASICINFO01BRO:
            memcpy(&Bat1_BasicInfo01, frame.data, sizeof(Bat1_BasicInfo01));
            return frame.id;
            break;

        case BAT1_BASICINFO00ACK:
        case BAT1_BASICINFO00BRO:
            memcpy(&Bat1_BasicInfo00, frame.data, sizeof(Bat1_BasicInfo00));
            return frame.id;
            break;

        case BAT1_MCUOPERATIONACK:
            memcpy(&Bat1_MCUOperation, frame.data, sizeof(Bat1_MCUOperation));
            return frame.id;
            break;

        case BAT1_DEBUGMODEACK:
            memcpy(&Bat1_DebugMode, frame.data, sizeof(Bat1_DebugMode));
            return frame.id;
            break;

        case BAT1_ERRORACK:
        case BAT1_ERRORBRO:
            memcpy(&Bat1_Error, frame.data, sizeof(Bat1_Error));
            return frame.id;
            break;

        case BAT1_WARNINGBRO:
            memcpy(&Bat1_Warning_BRO, frame.data, sizeof(Bat1_Warning_BRO));
            return frame.id;
            break;

        case BAT1_INFO00ACK:
        case BAT1_INFO00BRO:
            memcpy(&Bat1_Info00, frame.data, sizeof(Bat1_Info00));
            return frame.id;
            break;

        case BAT1_INFO01ACK:
        case BAT1_INFO01BRO:
            memcpy(&Bat1_Info01, frame.data, sizeof(Bat1_Info01));
            return frame.id;
            break;

        case BAT1_INFO02ACK:
        case BAT1_INFO02BRO:
            memcpy(&Bat1_Info02, frame.data, sizeof(Bat1_Info02));
            return frame.id;
            break;

        case BAT1_INFO03ACK:
        case BAT1_INFO03BRO:
            memcpy(&Bat1_Info03, frame.data, sizeof(Bat1_Info03));
            return frame.id;
            break;

        case BAT1_INFO04ACK:
        case BAT1_INFO04BRO:
            memcpy(&Bat1_Info04, frame.data, sizeof(Bat1_Info04));
            return frame.id;
            break;

        case BAT1_INFO05ACK:
        case BAT1_INFO05BRO:
            memcpy(&Bat1_Info05, frame.data, sizeof(Bat1_Info05));
            return frame.id;
            break;

        case BAT1_INFO06ACK:
        case BAT1_INFO06BRO:
            memcpy(&Bat1_Info06, frame.data, sizeof(Bat1_Info06));
            return frame.id;
            break;

        case BAT1_INFO07ACK:
        case BAT1_INFO07BRO:
            memcpy(&Bat1_Info07, frame.data, sizeof(Bat1_Info07));
            return frame.id;
            break;

        case BAT1_TESTMODEACK:
            memcpy(&Bat1_TestMode, frame.data, sizeof(Bat1_TestMode));
            return frame.id;
            break;

        default:
            return 0; // 返回 0 表示已處理 ACK
    }
}
// 發送請求

void bat1_processQueueTX_REQ(uint32_t CANID) {
    CanFrame frame;
    frame.id = CANID;

    switch (CANID) {
        case BAT1_BASICCOMMAND0REQ:
            frame.dlc = sizeof(Bat1_BasicCommand0_REQ);
            memcpy(frame.data, &Bat1_BasicCommand0_REQ, frame.dlc);
            break;

        case BAT1_BASICCOMMAND1REQ:
            frame.dlc = sizeof(Bat1_BasicCommand1_REQ);
            memcpy(frame.data, &Bat1_BasicCommand1_REQ, frame.dlc);
            break;

        case BAT1_BASICINFO00REQ:
            frame.dlc = sizeof(Bat1_BasicInfo00_REQ);
            memcpy(frame.data, &Bat1_BasicInfo00_REQ, frame.dlc);
            break;

        case BAT1_BASICINFO01REQ:
            frame.dlc = sizeof(Bat1_BasicInfo01_REQ);
            memcpy(frame.data, &Bat1_BasicInfo01_REQ, frame.dlc);
            break;

        case BAT1_BASICINFO02REQ:
            frame.dlc = sizeof(Bat1_BasicInfo02_REQ);
            memcpy(frame.data, &Bat1_BasicInfo02_REQ, frame.dlc);
            break;

        case BAT1_BASICINFO03REQ:
            frame.dlc = sizeof(Bat1_BasicInfo03_REQ);
            memcpy(frame.data, &Bat1_BasicInfo03_REQ, frame.dlc);
            break;

        case BAT1_DEBUGMODEREQ:
            frame.dlc = sizeof(Bat1_DebugMode_REQ);
            memcpy(frame.data, &Bat1_DebugMode_REQ, frame.dlc);
            break;

        case BAT1_ERRORREQ:
            frame.dlc = sizeof(Bat1_Error_REQ);
            memcpy(frame.data, &Bat1_Error_REQ, frame.dlc);
            break;

        case BAT1_INFO00REQ:
            frame.dlc = sizeof(Bat1_Info00_REQ);
            memcpy(frame.data, &Bat1_Info00_REQ, frame.dlc);
            break;

        case BAT1_INFO01REQ:
            frame.dlc = sizeof(Bat1_Info01_REQ);
            memcpy(frame.data, &Bat1_Info01_REQ, frame.dlc);
            break;

        case BAT1_INFO02REQ:
            frame.dlc = sizeof(Bat1_Info02_REQ);
            memcpy(frame.data, &Bat1_Info02_REQ, frame.dlc);
            break;

        case BAT1_INFO03REQ:
            frame.dlc = sizeof(Bat1_Info03_REQ);
            memcpy(frame.data, &Bat1_Info03_REQ, frame.dlc);
            break;

        case BAT1_INFO04REQ:
            frame.dlc = sizeof(Bat1_Info04_REQ);
            memcpy(frame.data, &Bat1_Info04_REQ, frame.dlc);
            break;

        case BAT1_INFO05REQ:
            frame.dlc = sizeof(Bat1_Info05_REQ);
            memcpy(frame.data, &Bat1_Info05_REQ, frame.dlc);
            break;

        case BAT1_INFO06REQ:
            frame.dlc = sizeof(Bat1_Info06_REQ);
            memcpy(frame.data, &Bat1_Info06_REQ, frame.dlc);
            break;

        case BAT1_INFO07REQ:
            frame.dlc = sizeof(Bat1_Info07_REQ);
            memcpy(frame.data, &Bat1_Info07_REQ, frame.dlc);
            break;

        case BAT1_MCUOPERATIONREQ:
            frame.dlc = sizeof(Bat1_MCUOperation_REQ);
            memcpy(frame.data, &Bat1_MCUOperation_REQ, frame.dlc);
            break;

        case BAT1_TESTMODEREQ:
            frame.dlc = sizeof(Bat1_TestMode_REQ);
            memcpy(frame.data, &Bat1_TestMode_REQ, frame.dlc);
            break;

        default:
            // ?��??��? CANID，�???
            return;
    }

    // ?�送數??
    enqueueTXQueue(&frame); // 發送數據
}
