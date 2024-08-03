#include "logic.h"
#include <algorithm>

static uint32_t lgic_c_numOfTriggeringPins_U32 = 0;
static float lgic_d_currentDutyCycles_F32[32] = {0};

void logic_compose(uint8_t* DataPtr, uint32_t* MsgIdPtr)
{
    // TODO: no-init guard
    uint16_t validatedValues[32] = {0};
    for (int i = 0; i < lgic_c_numOfTriggeringPins_U32; i++)
    {
        validatedValues[i] = uint16_t (lgic_d_currentDutyCycles_F32[i] * 1000.0f);
    }

    static uint8_t msgIndx = 0;
    if (0 == msgIndx)
    {
        *MsgIdPtr = 0x96;

        // pack lgic_d_currentDutyCycles_F32 into CanMsgPtr->data.u8
        DataPtr[0] = (validatedValues[0] & 0xFF);
        DataPtr[1] = ((validatedValues[1] & 0x3F) << 2) | ((validatedValues[0] >> 8) & 0x03);
        DataPtr[2] = ((validatedValues[2] & 0x0F) << 4) | ((validatedValues[1] >> 6) & 0x0F);
        DataPtr[3] = ((validatedValues[3] & 0x03) << 6) | ((validatedValues[2] >> 4) & 0x3F);
        DataPtr[4] = ((validatedValues[3] >> 2) & 0xFF);
        DataPtr[5] = (validatedValues[4] & 0xFF);
        DataPtr[6] = ((validatedValues[4] >> 8) & 0x03);
        DataPtr[7] = 0;

        msgIndx = 1;
    }
    else if (1 == msgIndx)
    {
        *MsgIdPtr = 0x97;

        // pack lgic_d_currentDutyCycles_F32 into canMsg0x97.data.u8
        DataPtr[0] = (validatedValues[5] & 0xFF);
        DataPtr[1] = ((validatedValues[6] & 0x3F) << 2) | ((validatedValues[5] >> 8) & 0x03);
        DataPtr[2] = ((validatedValues[7] & 0x0F) << 4) | ((validatedValues[6] >> 6) & 0x0F);
        DataPtr[3] = ((validatedValues[8] & 0x03) << 6) | ((validatedValues[7] >> 4) & 0x3F);
        DataPtr[4] = ((validatedValues[8] >> 2) & 0xFF);
        DataPtr[5] = (validatedValues[9] & 0xFF);
        DataPtr[6] = ((validatedValues[9] >> 8) & 0x03);
        DataPtr[7] = 0;

        msgIndx = 0;
    }
}