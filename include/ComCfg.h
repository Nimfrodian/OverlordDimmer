#ifndef COM_CFG_H
#define COM_CFG_H

#include <ESP32CAN.h>

typedef enum
{
    CAN_REPLY_MESSAGE0 = 0,
    CAN_REPLY_MESSAGE1,
    CAN_COMMAND_MESSAGE,
    NUM_OF_CAN_MSG
} ComCfg_canMsgIndxType;

#endif