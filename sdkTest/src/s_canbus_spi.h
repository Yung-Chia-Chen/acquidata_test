#ifndef _S_CANBUS_SPI_H_
#define _S_CANBUS_SPI_H_

typedef struct 
{
    unsigned long ID;
    unsigned char ext;
    unsigned char rtr;
    unsigned char DLC;
    unsigned char data[8];
}SPI_CAN_frame_t;

bool canbus_spi_init();
bool canbus_spi_transmit(SPI_CAN_frame_t* frame);
bool canbus_spi_receive(SPI_CAN_frame_t *frame, unsigned short u16timeout);

#endif