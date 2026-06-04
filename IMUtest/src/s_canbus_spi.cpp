#include <mcp_can.h>
#include <SPI.h>
#include "s_canbus_spi.h"
#include "codeHw.h"
// #include "chc_can_queue.h"

MCP_CAN CAN0(pinMcu_SPI_CANBUS_CS_1); // Set CS to pin 10
#define CAN0_INT pinMcu_CANBUS_INT_1

bool canbus_spi_init()
{
    // spi.begin(pinMcu_SPI_SCK, pinMcu_SPI_MISO, pinMcu_SPI_MOSI, pinMcu_SPI_CANBUS_CS_1);
    // spi.begin(13,12,11,10);
    // Initialize MCP2515 running at 16MHz with a baudrate of 500kb/s and the masks and filters disabled.
    if (CAN0.begin(MCP_ANY, CAN_250KBPS, MCP_16MHZ, pinMcu_SPI_SCK, pinMcu_SPI_MISO, pinMcu_SPI_MOSI, pinMcu_SPI_CANBUS_CS_1) == CAN_OK)
    {
        CAN0.setMode(MCP_NORMAL); // Change to normal mode to allow messages to be transmitted
        pinMode(CAN0_INT, INPUT); // Configuring pin for /INT input
        Serial.println("MCP2515 Initialized Successfully!");
        return true;
    }
    else
    {
        Serial.println("Error Initializing MCP2515...");
        return false;
    }
}

bool canbus_spi_transmit(SPI_CAN_frame_t *frame)
{
    uint8_t sndStat = CAN0.sendMsgBuf(frame->ID, frame->ext, frame->DLC, frame->data);
    if (sndStat == CAN_OK)
    {
        Serial.println("Message Sent Successfully!");
        return true;
    }
    else
    {
        Serial.println("Error Sending Message...");
        return false;
    }
}

bool canbus_spi_receive(SPI_CAN_frame_t *frame, unsigned short u16timeout)
{
    unsigned long ultimeoutTimer = millis();
    while (digitalRead(CAN0_INT))
    {
        if (millis() - ultimeoutTimer >= u16timeout)
        {
            return false;
        }
    }

    CAN0.readMsgBuf(&frame->ID, &frame->DLC, frame->data); // Read data: len = data length, buf = data byte(s)

    // if ((frame->ID & 0x80000000) == 0x80000000) // Determine if ID is standard (11 bits) or extended (29 bits)
    //     printf("Extended ID: 0x%2x  DLC: %1d  Data:", (frame->ID & 0x1FFFFFFF), frame->DLC);
    // else
    //     printf("Standard ID: 0x%2x     DLC: %1d  Data:", frame->ID, frame->DLC);

    // // Serial.print(msgString);

    // if ((frame->ID & 0x40000000) == 0x40000000)
    // { // Determine if message is a remote request frame.
    //     // sprintf(msgString, " REMOTE REQUEST FRAME");
    //     Serial.print("REMOTE REQUEST FRAME");
    // }
    // else
    // {
    //     for (byte i = 0; i < frame->DLC; i++)
    //     {
    //         printf(" 0x%2X", frame->data[i]);
    //         // Serial.print(msgString);
    //     }
    // }
    // Serial.println();
    return true;
}
