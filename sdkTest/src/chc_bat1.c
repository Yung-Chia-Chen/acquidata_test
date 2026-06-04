#include "chc_bat1.h"

BAT1_BASICCOMMAND0_REQ_T Bat1_BasicCommand0_REQ;
BAT1_BASICCOMMAND0_BRO_T Bat1_BasicCommand0_BRO;
BAT1_BASICCOMMAND1_REQ_T Bat1_BasicCommand1_REQ;
BAT1_BASICCOMMAND1_BRO_T Bat1_BasicCommand1_BRO;
BAT1_BASICINFO00_REQ_T Bat1_BasicInfo00_REQ = {BAT1_BASICINFO00_CYCLE_TIME};
BAT1_BASICINFO00_ACK_T Bat1_BasicInfo00;
BAT1_BASICINFO01_REQ_T Bat1_BasicInfo01_REQ = {BAT1_BASICINFO01_CYCLE_TIME};
BAT1_BASICINFO01_ACK_T Bat1_BasicInfo01;
BAT1_BASICINFO02_REQ_T Bat1_BasicInfo02_REQ = {BAT1_BASICINFO02_CYCLE_TIME};
BAT1_BASICINFO02_ACK_T Bat1_BasicInfo02;
BAT1_BASICINFO03_REQ_T Bat1_BasicInfo03_REQ = {BAT1_BASICINFO03_CYCLE_TIME};
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

void bat1_processQueueTX_ACK(S_CAN_QUEUE_INSTANCE* pQueue,uint32_t CAN_ID){
    switch (CAN_ID) {
        case BAT1_BASICINFO03REQ:
        case BAT1_BASICINFO02REQ:
        case BAT1_BASICINFO01REQ:
        case BAT1_BASICINFO00REQ:
            {
                CanFrame ack_msg;
                ack_msg.id = (CAN_ID == BAT1_BASICINFO03REQ) ? BAT1_BASICINFO03ACK :
                             (CAN_ID == BAT1_BASICINFO02REQ) ? BAT1_BASICINFO02ACK :
                             (CAN_ID == BAT1_BASICINFO01REQ) ? BAT1_BASICINFO01ACK : BAT1_BASICINFO00ACK;
                
                ack_msg.dlc = 8;
                if (CAN_ID == BAT1_BASICINFO03REQ) { memcpy(ack_msg.data, &Bat1_BasicInfo03, 8); }
                else if (CAN_ID == BAT1_BASICINFO02REQ) { memcpy(ack_msg.data, &Bat1_BasicInfo02, 8); }
                else if (CAN_ID == BAT1_BASICINFO01REQ) { memcpy(ack_msg.data, &Bat1_BasicInfo01, 8); }
                else { memcpy(ack_msg.data, &Bat1_BasicInfo00, 8); }
                vCanQueue_enqueue(pQueue,&ack_msg);
            }
            break;

        case BAT1_INFO00REQ:
            {
                CanFrame ack_msg;
                ack_msg.id = BAT1_INFO00ACK;
                ack_msg.dlc = 7;
                ack_msg.data[0] = (uint8_t)Bat1_Info00.EstimateRange;
                ack_msg.data[1] = (uint8_t)(Bat1_Info00.EstimateRange >> 8);
                ack_msg.data[2] = (Bat1_Info00.ChargeFETOn) | (Bat1_Info00.DischargeFETOff << 1) |
                                  (Bat1_Info00.Charging << 2) | (Bat1_Info00.Discharging << 3) |
                                  (Bat1_Info00.FullyCharged << 4) | (Bat1_Info00.FullyDischarged << 5) |
                                  (Bat1_Info00.NearlyDischarged << 6) | (Bat1_Info00.ChargerDetected << 7);
                ack_msg.data[3] = 0; 
                ack_msg.data[4] = (Bat1_Info00.BatteryConfiguration & 0x03);
                ack_msg.data[5] = Bat1_Info00.PreviousError0;
                ack_msg.data[6] = Bat1_Info00.PreviousError1;
                vCanQueue_enqueue(pQueue,&ack_msg);
            }
            break;
            
        case BAT1_ERRORREQ:
            {
                CanFrame ack_msg;
                ack_msg.id = BAT1_ERRORACK;
                ack_msg.dlc = 8;
                memcpy(ack_msg.data, &Bat1_Error, 8);
                vCanQueue_enqueue(pQueue,&ack_msg);
            }
            break;

        case BAT1_MCUOPERATIONREQ:
            {
                CanFrame ack_msg;
                ack_msg.id = BAT1_MCUOPERATIONACK;
                ack_msg.dlc = 2;
                memcpy(ack_msg.data, &Bat1_MCUOperation, 2);
                vCanQueue_enqueue(pQueue,&ack_msg);
            }
            break;

        default: break;
    }
}

void bat1_processQueueTX_BRO(S_CAN_QUEUE_INSTANCE* pQueue,uint32_t current_time) {
    static uint32_t last_send_time_BAT1_BASICINFO03BRO = 0;
    if (current_time - last_send_time_BAT1_BASICINFO03BRO >= Bat1_BasicInfo03_REQ.BroadcastPeriodChange) {
        CanFrame frame;
        frame.id = BAT1_BASICINFO03BRO;
        frame.dlc = 8;
        memcpy(frame.data, &Bat1_BasicInfo03, 8);
        vCanQueue_enqueue(pQueue,&frame);
        last_send_time_BAT1_BASICINFO03BRO = current_time;
    }

    static uint32_t last_send_time_BAT1_BASICINFO02BRO = 0;
    if (current_time - last_send_time_BAT1_BASICINFO02BRO >= Bat1_BasicInfo02_REQ.BroadcastPeriodChange) {
        CanFrame frame;
        frame.id = BAT1_BASICINFO02BRO;
        frame.dlc = 8;
        memcpy(frame.data, &Bat1_BasicInfo02, 8);
        vCanQueue_enqueue(pQueue,&frame);
        last_send_time_BAT1_BASICINFO02BRO = current_time;
    }

    static uint32_t last_send_time_BAT1_BASICINFO01BRO = 0;
    if (current_time - last_send_time_BAT1_BASICINFO01BRO >= Bat1_BasicInfo01_REQ.BroadcastPeriodChange) {
        CanFrame frame;
        frame.id = BAT1_BASICINFO01BRO;
        frame.dlc = 8;
        memcpy(frame.data, &Bat1_BasicInfo01, 8);
        vCanQueue_enqueue(pQueue,&frame);
        last_send_time_BAT1_BASICINFO01BRO = current_time;
    }

    static uint32_t last_send_time_BAT1_BASICINFO00BRO = 0;
    if (current_time - last_send_time_BAT1_BASICINFO00BRO >= Bat1_BasicInfo00_REQ.BroadcastPeriodChange) {
        CanFrame frame;
        frame.id = BAT1_BASICINFO00BRO;
        frame.dlc = 8;
        memcpy(frame.data, &Bat1_BasicInfo00, 8);
        vCanQueue_enqueue(pQueue,&frame);
        last_send_time_BAT1_BASICINFO00BRO = current_time;
    }

    static uint32_t last_send_time_BAT1_ERRORBRO = 0;
    if (current_time - last_send_time_BAT1_ERRORBRO >= BAT1_ERROR_CYCLE_TIME) {
        CanFrame frame;
        frame.id = BAT1_ERRORBRO;
        frame.dlc = 8;
        memcpy(frame.data, &Bat1_Error, 8);
        vCanQueue_enqueue(pQueue,&frame);
        last_send_time_BAT1_ERRORBRO = current_time;
    }

    static uint32_t last_send_time_BAT1_INFO07BRO = 0;
    if (current_time - last_send_time_BAT1_INFO07BRO >= BAT1_INFO07_CYCLE_TIME) {
        CanFrame frame;
        frame.id = BAT1_INFO07BRO;
        frame.dlc = 8;
        frame.data[0] = (uint8_t)Bat1_Info07.UTC;
        frame.data[1] = (uint8_t)(Bat1_Info07.UTC >> 8);
        frame.data[2] = (uint8_t)(Bat1_Info07.UTC >> 16);
        frame.data[3] = (uint8_t)(Bat1_Info07.UTC >> 24);
        memset(&frame.data[4], 0, 4);
        vCanQueue_enqueue(pQueue,&frame);
        last_send_time_BAT1_INFO07BRO = current_time;
    }

    static uint32_t last_send_time_BAT1_INFO06BRO = 0;
    if (current_time - last_send_time_BAT1_INFO06BRO >= BAT1_INFO06_CYCLE_TIME) {
        CanFrame frame;
        frame.id = BAT1_INFO06BRO;
        frame.dlc = 8;
        frame.data[0] = Bat1_Info06.TemperatureSensor1;
        frame.data[1] = Bat1_Info06.TemperatureSensor2;
        frame.data[2] = Bat1_Info06.TemperatureSensor3;
        frame.data[3] = Bat1_Info06.TemperatureSensor4;
        frame.data[4] = Bat1_Info06.OverTemperatureProtection;
        frame.data[5] = Bat1_Info06.UnderTemperatureProtection;
        memset(&frame.data[6], 0, 2);
        vCanQueue_enqueue(pQueue,&frame);
        last_send_time_BAT1_INFO06BRO = current_time;
    }

    static uint32_t last_send_time_BAT1_INFO05BRO = 0;
    if (current_time - last_send_time_BAT1_INFO05BRO >= BAT1_INFO05_CYCLE_TIME) {
        CanFrame frame;
        frame.id = BAT1_INFO05BRO;
        frame.dlc = 8;
        frame.data[0] = Bat1_Info05.MaxVoltage[0];
        frame.data[1] = Bat1_Info05.MaxVoltage[1];
        frame.data[2] = Bat1_Info05.MaxVoltage[2];
        frame.data[3] = 0;
        frame.data[4] = Bat1_Info05.OverVoltageProtection[0];
        frame.data[5] = Bat1_Info05.OverVoltageProtection[1];
        frame.data[6] = Bat1_Info05.OverVoltageProtection[2];
        frame.data[7] = 0;
        vCanQueue_enqueue(pQueue,&frame);
        last_send_time_BAT1_INFO05BRO = current_time;
    }

    static uint32_t last_send_time_BAT1_INFO04BRO = 0;
    if (current_time - last_send_time_BAT1_INFO04BRO >= BAT1_INFO04_CYCLE_TIME) {
        CanFrame frame;
        frame.id = BAT1_INFO04BRO;
        frame.dlc = 8;
        memcpy(frame.data, &Bat1_Info04, 8);
        vCanQueue_enqueue(pQueue,&frame);
        last_send_time_BAT1_INFO04BRO = current_time;
    }

    static uint32_t last_send_time_BAT1_INFO03BRO = 0;
    if (current_time - last_send_time_BAT1_INFO03BRO >= BAT1_INFO03_CYCLE_TIME) {
        CanFrame frame;
        frame.id = BAT1_INFO03BRO;
        frame.dlc = 8;
        frame.data[0] = (uint8_t)Bat1_Info03.ProductionDate;
        frame.data[1] = (uint8_t)(Bat1_Info03.ProductionDate >> 8);
        frame.data[2] = (uint8_t)(Bat1_Info03.ProductionDate >> 16);
        frame.data[3] = (uint8_t)(Bat1_Info03.ProductionDate >> 24);
        frame.data[4] = (uint8_t)Bat1_Info03.ChargeCycleCount;
        frame.data[5] = (uint8_t)(Bat1_Info03.ChargeCycleCount >> 8);
        frame.data[6] = (uint8_t)Bat1_Info03.LongestDaysNoCharge; // bits 0-7
        frame.data[7] = ((uint8_t)(Bat1_Info03.LongestDaysNoCharge >> 8) & 0x07) | 
                        (Bat1_Info03.ChargingStatus << 3); 
        vCanQueue_enqueue(pQueue,&frame);
        last_send_time_BAT1_INFO03BRO = current_time;
    }

    static uint32_t last_send_time_BAT1_INFO02BRO = 0;
    if (current_time - last_send_time_BAT1_INFO02BRO >= BAT1_INFO02_CYCLE_TIME) {
        CanFrame frame;
        frame.id = BAT1_INFO02BRO;
        frame.dlc = 8;
        memcpy(frame.data, &Bat1_Info02, 8);
        vCanQueue_enqueue(pQueue,&frame);
        last_send_time_BAT1_INFO02BRO = current_time;
    }

    static uint32_t last_send_time_BAT1_INFO01BRO = 0;
    if (current_time - last_send_time_BAT1_INFO01BRO >= BAT1_INFO01_CYCLE_TIME) {
        CanFrame frame;
        frame.id = BAT1_INFO01BRO;
        frame.dlc = 8;
        frame.data[0] = Bat1_Info01.ActualVoltage[0];
        frame.data[1] = Bat1_Info01.ActualVoltage[1];
        frame.data[2] = Bat1_Info01.ActualVoltage[2];
        frame.data[3] = 0;
        frame.data[4] = (uint8_t)Bat1_Info01.ActualCurrent;
        frame.data[5] = (uint8_t)(Bat1_Info01.ActualCurrent >> 8);
        frame.data[6] = Bat1_Info01.RelativeStateOfCharge;
        frame.data[7] = Bat1_Info01.RelativeStateOfHealthy;
        vCanQueue_enqueue(pQueue,&frame);
        last_send_time_BAT1_INFO01BRO = current_time;
    }

    static uint32_t last_send_time_BAT1_INFO00BRO = 0;
    if (current_time - last_send_time_BAT1_INFO00BRO >= BAT1_INFO00_CYCLE_TIME) {
        CanFrame frame;
        frame.id = BAT1_INFO00BRO;
        frame.dlc = 7;
        frame.data[0] = (uint8_t)Bat1_Info00.EstimateRange;
        frame.data[1] = (uint8_t)(Bat1_Info00.EstimateRange >> 8);
        frame.data[2] = (Bat1_Info00.ChargeFETOn) | (Bat1_Info00.DischargeFETOff << 1) |
                        (Bat1_Info00.Charging << 2) | (Bat1_Info00.Discharging << 3) |
                        (Bat1_Info00.FullyCharged << 4) | (Bat1_Info00.FullyDischarged << 5) |
                        (Bat1_Info00.NearlyDischarged << 6) | (Bat1_Info00.ChargerDetected << 7);
        frame.data[3] = 0; 
        frame.data[4] = (Bat1_Info00.BatteryConfiguration & 0x03);
        frame.data[5] = Bat1_Info00.PreviousError0;
        frame.data[6] = Bat1_Info00.PreviousError1;
        vCanQueue_enqueue(pQueue,&frame);
        last_send_time_BAT1_INFO00BRO = current_time;
    }
}

uint32_t bat1_processQueueRX_REQ(CanFrame frame) {
    switch (frame.id) {
        case BAT1_BASICINFO03REQ:
            memcpy(&Bat1_BasicInfo03_REQ, frame.data, frame.dlc);
            return BAT1_BASICINFO03REQ;

        case BAT1_BASICINFO02REQ:
            memcpy(&Bat1_BasicInfo02_REQ, frame.data, frame.dlc);
            return BAT1_BASICINFO02REQ;

        case BAT1_BASICINFO01REQ:
            memcpy(&Bat1_BasicInfo01_REQ, frame.data, frame.dlc);
            return BAT1_BASICINFO01REQ;

        case BAT1_BASICINFO00REQ:
            memcpy(&Bat1_BasicInfo00_REQ, frame.data, frame.dlc);
            return BAT1_BASICINFO00REQ;

        case BAT1_MCUOPERATIONREQ:
            memcpy(&Bat1_MCUOperation_REQ, frame.data, frame.dlc);
            return BAT1_MCUOPERATIONREQ;

        case BAT1_DEBUGMODEREQ:
            memcpy(&Bat1_DebugMode_REQ, frame.data, frame.dlc);
            return BAT1_DEBUGMODEREQ;

        case BAT1_ERRORREQ:
            return BAT1_ERRORREQ;

        case BAT1_TESTMODEREQ:
            memcpy(&Bat1_TestMode_REQ, frame.data, frame.dlc);
            return BAT1_TESTMODEREQ;

        case BAT1_INFO07REQ: return BAT1_INFO07REQ;
        case BAT1_INFO06REQ: return BAT1_INFO06REQ;
        case BAT1_INFO05REQ: return BAT1_INFO05REQ;
        case BAT1_INFO04REQ: return BAT1_INFO04REQ;
        case BAT1_INFO03REQ: return BAT1_INFO03REQ;
        case BAT1_INFO02REQ: return BAT1_INFO02REQ;
        case BAT1_INFO01REQ: return BAT1_INFO01REQ;
        case BAT1_INFO00REQ: return BAT1_INFO00REQ;

        default:
            return 0;
    }
}
