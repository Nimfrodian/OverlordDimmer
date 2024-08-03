#ifndef CAN_COMMS_H
#define CAN_COMMS_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ComCfg.h"

typedef struct
{
    uint32_t timeRequest_10ms;
    uint32_t dutyCycleReq;
    uint32_t triggerIndexMask;
    bool dataReceived;
} canRxDataType;

void ComCan_init(void);
void ComCan_receive(void);
void ComCan_transmit(CAN_frame_t* CanMsgPtr, uint8_t count);
canRxDataType* ComCan_get_rxData(void);

#endif