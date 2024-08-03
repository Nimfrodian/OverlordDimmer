#include "logic.h"
#include <algorithm>

// GLOBAL VARIABLES //////////////////////////////////////////

static bool lgic_f_moduleInit_B = false;
static uint32_t lgic_c_numOfTriggeringPins_U32 = 0;  // number of physical triggering pins
static dutyCycleStructType lgic_s_dutyCycleConfig_S[32];

//////////////////////////////////////////////////////////////
// FUNCTIONS /////////////////////////////////////////////////
void logic_init(logicInitType* logicCfg)
{
    lgic_c_numOfTriggeringPins_U32 = logicCfg->numOfPhyOut;
    lgic_f_moduleInit_B = true;
}

void config_duty_cycle(uint32_t OutIndx, float EndPrcnt, uint64_t TimeToEndPrcnt_ms)
{
    if (!lgic_f_moduleInit_B)
    {
        // TODO: report ERROR
    }
    else
    {
        if (TimeToEndPrcnt_ms < 10) {TimeToEndPrcnt_ms = 10;}
        uint64_t timeToEndPrcnt_10ms = TimeToEndPrcnt_ms / 10;
        if (EndPrcnt > 1.0f) {EndPrcnt = 1.0f;}
        if (EndPrcnt < 0.0f) {EndPrcnt = 0.0f;}
        if (timeToEndPrcnt_10ms > 8640000) {timeToEndPrcnt_10ms = 8640000;}
        lgic_s_dutyCycleConfig_S[OutIndx].endVal = (int64_t) (EndPrcnt * 1000000000.0f);
        lgic_s_dutyCycleConfig_S[OutIndx].delta = (((int64_t)lgic_s_dutyCycleConfig_S[OutIndx].endVal - (int64_t)lgic_s_dutyCycleConfig_S[OutIndx].outVal) / (int64_t)timeToEndPrcnt_10ms);   // 10^9 bigger
    }
}

void logic_compose(uint8_t* DataPtr, uint32_t* MsgIdPtr)
{
    if (!lgic_f_moduleInit_B)
    {
        // TODO: report ERROR
    }
    else
    {
        uint16_t validatedValues[32] = {0};
        for (int i = 0; i < lgic_c_numOfTriggeringPins_U32; i++)
        {
            validatedValues[i] = uint16_t (lgic_s_dutyCycleConfig_S[i].currDutCycl * 1000.0f);
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
}

