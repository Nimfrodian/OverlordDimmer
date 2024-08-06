#include "ComCan.h"
#include "lgic.h"

CAN_device_t CAN_cfg;
static canRxDataType g_rxData =
{
    .timeRequest_10ms = 0,
    .dutyCycleReq = 0,
    .triggerIndexMask = 0,
    .dataReceived = 0,
};

void ComCan_init()
{
    CAN_cfg.speed = CAN_SPEED_250KBPS;
    CAN_cfg.tx_pin_id = GPIO_NUM_5;
    CAN_cfg.rx_pin_id = GPIO_NUM_4;
    /* create a queue for CAN receiving */
    CAN_cfg.rx_queue = xQueueCreate(10,sizeof(CAN_frame_t));
    //initialize CAN Module
    ESP32Can.CANInit();
}

static void ComCan_parseMsg(CAN_frame_t* sourceMsgPtr)
{
    g_rxData.timeRequest_10ms = ((sourceMsgPtr->data.u8[2]& 0xFF) << 16) | ((sourceMsgPtr->data.u8[1] & 0xFF) << 8) | ((sourceMsgPtr->data.u8[0] & 0xFF) << 0);
    uint32_t dutyReq = ((sourceMsgPtr->data.u8[4] & 0x03) << 8) | ((sourceMsgPtr->data.u8[3] & 0xFF) << 0);
    g_rxData.dutyCycleReq = (dutyReq > 1000) ? 1000 : dutyReq;
    g_rxData.triggerIndexMask = ((sourceMsgPtr->data.u8[7] & 0xFF) << 16) | ((sourceMsgPtr->data.u8[6] & 0xFF) << 8) | ((sourceMsgPtr->data.u8[5] & 0xFF) << 0);
    g_rxData.dataReceived = 1;
}

/**
 * @brief Function reads and uses module parse on received message
 * @return (void)
 */
void ComCan_receive(void)
{
    CAN_frame_t _rxFrame;
    if (pdTRUE == xQueueReceive(CAN_cfg.rx_queue, &_rxFrame, 3*portTICK_PERIOD_MS))
    {
        uint32_t rxId = _rxFrame.MsgID;
        switch (rxId)
        {
            case 0x95: // message
            {
                ComCan_parseMsg(&_rxFrame);
            }
            break;

            default:
            break;
        }
    }
}

void ComCan_transmit(CAN_frame_t* CanMsgPtr, uint8_t Count)
{
    for (uint8_t msgArrIndx = 0; msgArrIndx < Count; msgArrIndx++)
    {
        ESP32Can.CANWriteFrame(&CanMsgPtr[msgArrIndx]);
    }
}

canRxDataType* ComCan_get_rxData(void)
{
    return &g_rxData;
}