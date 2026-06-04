

#include "CAN_base.h"

#ifdef CAN_lib_1
#include "driver/gpio.h"
#include "driver/twai.h"
#endif

#ifdef CAN_lib_2
// #include <CAN_config.h>
// #include <ESP32CAN.h>
CAN_device_t CAN_cfg;
const int rx_queue_size = 10;
#endif

#ifdef CAN_lib_3
#include <driver/can.h>

#endif
int iPinCanRx = 0;
int iPinCanTx = 0;
long lBaudrate = 0;

void CAN_base_setPin(int pinCanRx, int pinCanTx, long baudrate)
{
    iPinCanRx = pinCanRx;
    iPinCanTx = pinCanTx;
    lBaudrate = baudrate;
}
bool CAN_base_init()
{
#ifdef CAN_lib_1
    twai_stop();
    twai_driver_uninstall();
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_250KBITS();
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(
        (gpio_num_t)iPinCanTx,
        (gpio_num_t)iPinCanRx,
        TWAI_MODE_NORMAL); // TWAI_MODE_NORMAL, TWAI_MODE_NO_ACK or TWAI_MODE_LISTEN_ONLY
                           //
    switch (lBaudrate)
    {
    case 125000:
        break;
    case 250000:
        t_config = TWAI_TIMING_CONFIG_250KBITS();
        break;
    case 500000:
        t_config = TWAI_TIMING_CONFIG_500KBITS();
        break;
    default:
        break;
    }
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    esp_err_t err = twai_driver_install(&g_config, &t_config, &f_config);
    if (err == ESP_OK)
    {
        CAN_LOG_I("Driver installed");
    }
    else
    {
        CAN_LOG_I("Failed to install driver");
        // return err;
        return false;
    }
    err = twai_start();
    if (err == ESP_OK)
    {
        CAN_LOG_I("Driver started");
    }
    else
    {
        CAN_LOG_I("Failed to start driver");
        // return err;
        return false;
    }
    //     return err;
    return true;

#endif
#ifdef CAN_lib_2
    static uint8_t canInitialized = 0;
    // if (canInitialized == 1) {
    ESP32Can.CANStop();
    // }
    // long lBaudrate = 500000;
    switch (baudrate)
    {
    case 125000:
        // CAN_cfg.speed = CAN_SPEED_125KBPS;
        CAN_cfg.speed = CAN_SPEED_250KBPS;
        break;
    case 250000:
        // CAN_cfg.speed = CAN_SPEED_250KBPS;
        CAN_cfg.speed = CAN_SPEED_500KBPS;
        break;
    case 500000:
        // CAN_cfg.speed = CAN_SPEED_500KBPS;
        CAN_cfg.speed = CAN_SPEED_1000KBPS;
        break;
    case 10000000:
        // CAN_cfg.speed = CAN_SPEED_1000KBPS;
        break;
    default:
        break;
    }
    CAN_cfg.tx_pin_id = (gpio_num_t)pinCanTx;
    CAN_cfg.rx_pin_id = (gpio_num_t)pinCanRx;
    CAN_cfg.rx_queue = xQueueCreate(rx_queue_size, sizeof(CAN_frame_t));
    ESP32Can.CANInit();
    canInitialized = 1;
    return true;
#endif
#ifdef CAN_lib_3
    // Initialize configuration structures using macro initializers
    can_general_config_t g_config = CAN_GENERAL_CONFIG_DEFAULT(
        (gpio_num_t)pinCanRx,
        (gpio_num_t)pinCanTx,
        CAN_MODE_NORMAL);
    can_timing_config_t t_config = CAN_TIMING_CONFIG_500KBITS();
    can_filter_config_t f_config = CAN_FILTER_CONFIG_ACCEPT_ALL();

    // Install CAN driver
    if (can_driver_install(&g_config, &t_config, &f_config) == ESP_OK)
    {
        // printf("Driver installed\n");
        CAN_LOG_I("Driver installed");
    }
    else
    {
        // printf("Failed to install driver\n");
        CAN_LOG_I("Failed to install driver")
        return;
    }

    // Start CAN driver
    if (can_start() == ESP_OK)
    {
        // printf("Driver started\n");
        CAN_LOG_I("Driver started");
    }
    else
    {
        // printf("Failed to start driver\n");
        CAN_LOG_I("Failed to start driver");
        return;
    }
#endif
}

bool CAN_base_transmit(CAN_frame_t *CANFrame)
{
#ifdef CAN_lib_1
    esp_err_t err = twai_transmit((twai_message_t *)CANFrame, 0);
    if (err == ESP_OK)
    {
        CAN_LOG_I("Message queued for transmission");
    }
    else
    {
        switch(err)
        {
            case ESP_ERR_INVALID_STATE:
                CAN_LOG_E("TWAI_STATE_STOPPED");
                CAN_base_init();
                break;
            default:
                break;
        }
        String err_s = esp_err_to_name(err);
        CAN_LOG_I("Failed to queue message for transmission: %s", err_s.c_str());
        return false;
    }
    return true;
#endif
#ifdef CAN_lib_2
    if (ESP32Can.CANWriteFrame(CANFrame) != 0)
        return false;
    return true;
#endif
#ifdef CAN_lib_3
    // Configure message to transmit
    can_message_t message;
    message.identifier = CANFrame->identifier; // 0xAAAA;
    message.flags = CANFrame->extd             // CAN_MSG_FLAG_EXTD;
                        message.data_length_code = 4;
    for (int i = 0; i < 4; i++)
    {
        message.data[i] = 0;
    }

    // Queue message for transmission
    if (can_transmit(&message, pdMS_TO_TICKS(1000)) == ESP_OK)
    {
        // printf("Message queued for transmission\n");
        CAN_LOG_I("Message queued for transmission");
    }
    else
    {
        // printf("Failed to queue message for transmission\n");
        CAN_LOG_I("Failed to queue message for transmission");
    }
#endif
}

bool CAN_base_receive(CAN_frame_t *CANFrame, long timeout_ms)
{
#ifdef CAN_lib_1
    twai_status_info_t status_info;
    esp_err_t err_status = twai_get_status_info(&status_info);
    esp_err_t err;
    if (err_status != ESP_OK)
    {
        CAN_LOG_E("twai driver not installed");
        return false;
    }
    switch (status_info.state)
    {
    case TWAI_STATE_STOPPED:
        twai_start();
        CAN_LOG_E("TWAI_STATE_STOPPED");
        return false;
        break;
    case TWAI_STATE_RUNNING:
        err = twai_receive((twai_message_t *)CANFrame, timeout_ms);
        if (err == ESP_OK)
        {
            CAN_LOG_I("Message received");
        }
        else if (err == ESP_ERR_TIMEOUT)
        {
            // CAN_LOG_E("Receive timeout");
            return false;
        }
        else
        {
            String err_s = esp_err_to_name(err);
            CAN_LOG_E("Failed to receive message: %s", err_s.c_str());
            return false;
        }
        return true;
        break;
    case TWAI_STATE_BUS_OFF:
        twai_initiate_recovery();
        CAN_base_init();
        CAN_LOG_E("TWAI_STATE_BUS_OFF");
        return false;
        break;
    case TWAI_STATE_RECOVERING:
        CAN_LOG_E("TWAI_STATE_RECOVERING");
        return false;
        break;
    default:
        CAN_LOG_E("Unknown state");
        return false;
        break;
    }
#endif
#ifdef CAN_lib_2
    if (xQueueReceive(CAN_cfg.rx_queue, CANFrame, 3 * portTICK_PERIOD_MS) == pdTRUE)
    {

        if (CANFrame->FIR.B.FF == CAN_frame_std)
        {
            CAN_LOG_I("New standard frame");
        }
        else
        {
            CAN_LOG_I("New extended frame");
        }

        if (CANFrame->FIR.B.RTR == CAN_RTR)
        {
            CAN_LOG_I(" RTR from 0x%08X, DLC %d\r\n",
                      CANFrame->MsgID,
                      CANFrame->FIR.B.DLC);
        }
        else
        {
            CAN_LOG(" from 0x%08X, DLC %d, Data ",
                    CANFrame->MsgID,
                    CANFrame->FIR.B.DLC);
            for (int i = 0; i < CANFrame->FIR.B.DLC; i++)
            {
                CAN_LOG_S("0x%02X ", CANFrame->data.u8[i]);
            }
            CAN_LOG_S("\n");
        }
        return true;
    }
    return false;
#endif
#ifdef CAN_lib_3

    // Wait for message to be received
    can_message_t message;
    // if (can_receive(&message, pdMS_TO_TICKS(10000)) == ESP_OK) {
    if (can_receive(&message, pdMS_TO_TICKS(timeout_ms)) == ESP_OK)
    {
        // printf("Message received\n");
        CAN_LOG_I("Message received");
    }
    else
    {
        // printf("Failed to receive message\n");
        CAN_LOG_I("Failed to receive message");
        return;
    }

    // Process received message
    if (message.flags & CAN_MSG_FLAG_EXTD)
    {
        // printf("Message is in Extended Format\n");
        CAN_LOG_I("Message is in Extended Format");
    }
    else
    {
        // printf("Message is in Standard Format\n");
        CAN_LOG_I("Message is in Standard Format");
    }
    // printf("ID is %d\n", message.identifier);
    CAN_LOG_I("ID is %d", message.identifier);
    if (!(message.flags & CAN_MSG_FLAG_RTR))
    {
        for (int i = 0; i < message.data_length_code; i++)
        {
            // printf("Data byte %d = %d\n", i, message.data[i]);
            CAN_LOG_I("Data byte %d = %d", i, message.data[i]);
        }
    }
#endif
}
