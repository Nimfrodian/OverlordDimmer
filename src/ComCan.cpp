#include "ComCan.h"
#include "logic.h"

CAN_device_t CAN_cfg;
static canRxDataType g_rxData = {0};

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
    g_rxData.dutyCycleReq = ((sourceMsgPtr->data.u8[4] & 0x03) << 8)| ((sourceMsgPtr->data.u8[3] & 0xFF) << 0);
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

void ComCan_transmit(float* CurrentStatesArr, uint8_t NumOfTriggers)
{
    uint16_t validatedValues[32] = {0};
    for (int i = 0; i < NumOfTriggers; i++)
    {
        validatedValues[i] = uint16_t (CurrentStatesArr[i] * 1000.0f);
    }

    static uint8_t msgIndx = CAN_REPLY_MESSAGE0;
    if (CAN_REPLY_MESSAGE0 == msgIndx)
    {
        CAN_frame_t canMsg0x96 = {0};

        canMsg0x96.MsgID = 0x96;
        canMsg0x96.FIR.B.DLC = 8;
        canMsg0x96.FIR.B.FF = CAN_frame_std;
        canMsg0x96.FIR.B.RTR = CAN_no_RTR;

        // pack CurrentStatesArr into canMsg0x96.data.u8
        canMsg0x96.data.u8[0] = (validatedValues[0] & 0xFF);
        canMsg0x96.data.u8[1] = ((validatedValues[1] & 0x3F) << 2) | ((validatedValues[0] >> 8) & 0x03);
        canMsg0x96.data.u8[2] = ((validatedValues[2] & 0x0F) << 4) | ((validatedValues[1] >> 6) & 0x0F);
        canMsg0x96.data.u8[3] = ((validatedValues[3] & 0x03) << 6) | ((validatedValues[2] >> 4) & 0x3F);
        canMsg0x96.data.u8[4] = ((validatedValues[3] >> 2) & 0xFF);
        canMsg0x96.data.u8[5] = (validatedValues[4] & 0xFF);
        canMsg0x96.data.u8[6] = ((validatedValues[4] >> 8) & 0x03);
        canMsg0x96.data.u8[7] = 0;

        ESP32Can.CANWriteFrame(&canMsg0x96);
        msgIndx = CAN_REPLY_MESSAGE1;
    }
    else if (CAN_REPLY_MESSAGE1 == msgIndx)
    {
        CAN_frame_t canMsg0x97 = {0};

        canMsg0x97.MsgID = 0x97;
        canMsg0x97.FIR.B.DLC = 8;
        canMsg0x97.FIR.B.FF = CAN_frame_std;
        canMsg0x97.FIR.B.RTR = CAN_no_RTR;

        // pack CurrentStatesArr into canMsg0x97.data.u8
        canMsg0x97.data.u8[0] = (validatedValues[5] & 0xFF);
        canMsg0x97.data.u8[1] = ((validatedValues[6] & 0x3F) << 2) | ((validatedValues[5] >> 8) & 0x03);
        canMsg0x97.data.u8[2] = ((validatedValues[7] & 0x0F) << 4) | ((validatedValues[6] >> 6) & 0x0F);
        canMsg0x97.data.u8[3] = ((validatedValues[8] & 0x03) << 6) | ((validatedValues[7] >> 4) & 0x3F);
        canMsg0x97.data.u8[4] = ((validatedValues[8] >> 2) & 0xFF);
        canMsg0x97.data.u8[5] = (validatedValues[9] & 0xFF);
        canMsg0x97.data.u8[6] = ((validatedValues[9] >> 8) & 0x03);
        canMsg0x97.data.u8[7] = 0;

        ESP32Can.CANWriteFrame(&canMsg0x97);
        msgIndx = CAN_REPLY_MESSAGE0;
    }
}

canRxDataType* ComCan_get_rxData(void)
{
    return &g_rxData;
}