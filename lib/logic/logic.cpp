#include "logic.h"
#include <algorithm>

#define TRIGGER_DELAY_TABLE_SIZE 1001
#define MAX_TRIGGER_DELAY (9900)

// GLOBAL VARIABLES //////////////////////////////////////////

static bool lgic_f_moduleInit_B = false;
static uint32_t lgic_c_numOfTriggeringPins_U32 = 0;                     ///< number of physical triggering pins
static dutyCycleStructType lgic_s_dutyCycleConfig_S[32];
uint16_t lgic_ti_triggerDelayLookupTable_U16[TRIGGER_DELAY_TABLE_SIZE]; ///< given a duty cycle in 0.1% resolution (i.e., 100 is 10%) returns time it takes to turn off output to reach desired duty cycle

//////////////////////////////////////////////////////////////
// FUNCTIONS /////////////////////////////////////////////////
void logic_init(logicInitType* logicCfg)
{
    if (NULL == logicCfg)
    {
        // TODO: report ERROR
    }
    else
    {
        lgic_c_numOfTriggeringPins_U32 = logicCfg->numOfPhyOut;
        for (uint32_t i = 0; i < TRIGGER_DELAY_TABLE_SIZE; i++)
        {
            float prcntReq = ((float) i) / ((float)(TRIGGER_DELAY_TABLE_SIZE - 1));
            uint16_t delayTime = 10000 * (acosf(-2.0 * prcntReq + 1) / M_PI);
            if (delayTime > 10000)
            {
                delayTime = 10000;
            }
            lgic_ti_triggerDelayLookupTable_U16[i] = delayTime;
        }
        lgic_f_moduleInit_B = true;
    }
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


void calc_new_table(triggerTableType* PreparingTablePtr, float PeriodRatio)
{
    // clean triggering table
    triggerTableType tableEmptyData =
    {
        .deltaTimeToNext_us = 0,
        .triggerTime_us = 0,
        .mask = 0,
    };
    for (uint8_t i = 0; i < (lgic_c_numOfTriggeringPins_U32 + 1); i++)
    {
        PreparingTablePtr[i] = tableEmptyData;
    }

    // prepare table
    {
        // create triggering masks.
        // mask 0 is initial mask
        // each subsequent mask will turn off one or more physical outputs
        // last mask will have MSB set to signal it is the last to be applied
        {
            // sets initial mask
            {
                // set all bits to true that will have their duty cycle greater than 0 this period
                // e.g., 0b 0011 0000 0101 if outputs 0,2,8, and 9 should be active this period
                for (uint8_t i = 0; i < lgic_c_numOfTriggeringPins_U32; i++)
                {
                    if (0.0f < lgic_s_dutyCycleConfig_S[i].currDutCycl)
                    {
                        PreparingTablePtr[0].mask |= (0x01 << i);
                    }
                }
            }

            // sets each subsequent mask
            {
                // sets the output that is to be turned off to 0 in a copied initial mask
                // e.g., 0b 0011 0000 0100 if output 0 is to be turned off
                {
                    for (uint8_t i = 0; i < lgic_c_numOfTriggeringPins_U32; i++)
                    {
                        uint16_t dutyCyclIndx = (uint16_t) (1000.0 * lgic_s_dutyCycleConfig_S[i].currDutCycl);
                        float triggerTime_us = ((float) lgic_ti_triggerDelayLookupTable_U16[dutyCyclIndx]);

                        PreparingTablePtr[i+1].triggerTime_us = ((float) triggerTime_us) * PeriodRatio;

                        // turn off only if triggering time is not too close to end of period
                        if (MAX_TRIGGER_DELAY > triggerTime_us)
                        {
                            uint32_t turnOffMask = ~((0x01 << i)); // sets last mask flag to 0 as well as this output
                            PreparingTablePtr[i+1].mask = PreparingTablePtr[0].mask & turnOffMask;
                        }
                        else
                        {
                            PreparingTablePtr[i+1].mask = PreparingTablePtr[0].mask;   // Don't change mask and flag as final
                        }
                    }
                }
            }

            // sorts the table by triggering time
            {
                std::sort(&PreparingTablePtr[0], &PreparingTablePtr[lgic_c_numOfTriggeringPins_U32 + 1], [](triggerTableType &a, triggerTableType &b){return a.triggerTime_us < b.triggerTime_us;});
            }

            // compress the table
            {
                uint8_t compressed = 0; // this one is being compressed into
                uint8_t observed;   // this one is being observed if it is to be compressed or if it is unique

                for (observed = 1; observed < (lgic_c_numOfTriggeringPins_U32 + 1); observed++)
                {
                    if (PreparingTablePtr[observed].triggerTime_us > 0) // skip to the first member actually delayed
                    {
                        if (PreparingTablePtr[compressed].triggerTime_us == PreparingTablePtr[observed].triggerTime_us)
                        {
                            PreparingTablePtr[compressed].mask &= (PreparingTablePtr[observed].mask);
                        }
                        else
                        {
                            compressed++;
                            PreparingTablePtr[compressed].triggerTime_us = PreparingTablePtr[observed].triggerTime_us;
                            PreparingTablePtr[compressed].mask = PreparingTablePtr[compressed - 1].mask & (PreparingTablePtr[observed].mask);
                            PreparingTablePtr[compressed - 1].deltaTimeToNext_us = PreparingTablePtr[compressed].triggerTime_us - PreparingTablePtr[compressed - 1].triggerTime_us;
                            uint16_t maxTriggerTime = MAX_TRIGGER_DELAY * PeriodRatio;
                            if (PreparingTablePtr[compressed].triggerTime_us >= maxTriggerTime)
                            {
                                PreparingTablePtr[compressed-1].mask |= LAST_MASK_FLAG;   // if this one is to be triggered after max trigger delay, mark previous as last
                            }
                        }
                    }
                }
                PreparingTablePtr[compressed].mask |= LAST_MASK_FLAG;   // flag that the last one to be compressed into is the last one to trigger (possibly redundant)
            }
        }
    }
}