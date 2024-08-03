#include "logic.h"
#include <algorithm>

#define TRIGGER_DELAY_TABLE_SIZE 1001

static uint32_t g_numOfTriggers = 0;
static float g_zeroCrossDelay_us = 0.0f;
static dutyCycleConfigType g_dutyCycleConfig[32];
static float g_dutyCycles[32] = {0};  // holds duty cycles as calculated by calc_duty_cycle
static uint32_t g_lastTriggerTime_us = 0;

uint16_t g_triggerDelayLookupTable_us[TRIGGER_DELAY_TABLE_SIZE];

void init_logic(logicConfigType const* logicCfgPtr)
{
    g_numOfTriggers = (uint32_t) logicCfgPtr->numOfTriggers;
    g_zeroCrossDelay_us = (float) logicCfgPtr->zeroCrossDelay_us;

    initLookupTable();
}

void calc_duty_cycle(void)
{
    for (uint32_t i = 0; i < (g_numOfTriggers - 1); i++)
    {
        int64_t diff = (g_dutyCycleConfig[i].delta + g_dutyCycleConfig[i].outVal);
        if (g_dutyCycleConfig[i].delta > 0)
        {
            if (diff > g_dutyCycleConfig[i].endVal)
            {
                g_dutyCycleConfig[i].outVal = g_dutyCycleConfig[i].endVal;
            }
            else
            {
                g_dutyCycleConfig[i].outVal += g_dutyCycleConfig[i].delta;
            }
        }
        else
        {
            if (diff < g_dutyCycleConfig[i].endVal)
            {
                g_dutyCycleConfig[i].outVal = g_dutyCycleConfig[i].endVal;
            }
            else
            {
                g_dutyCycleConfig[i].outVal += g_dutyCycleConfig[i].delta;
            }
        }
        float dutCycle = ((float) g_dutyCycleConfig[i].outVal) / 1000000000.0f;
        if (dutCycle > 1.0f)
        {
            dutCycle = 1.0f;
        }
        else if (dutCycle < 0.0f)
        {
            dutCycle = 0.0f;
        }
        g_dutyCycles[i] = dutCycle;
    }
}

void initLookupTable()
{
    for (uint32_t i = 0; i < TRIGGER_DELAY_TABLE_SIZE; i++)
    {
        float prcntReq = ((float) i) / ((float)(TRIGGER_DELAY_TABLE_SIZE - 1));
        uint16_t delayTime = 10000 * (acosf(-2.0 * prcntReq + 1) / M_PI);
        if (delayTime > 9900)
        {
            delayTime = 9900;
        }
        g_triggerDelayLookupTable_us[i] = delayTime;
    }
}

void config_duty_cycle(uint32_t OutIndx, float EndPrcnt, uint64_t TimeToEndPrcnt_ms)
{
    if (TimeToEndPrcnt_ms < 10) {TimeToEndPrcnt_ms = 10;}
    uint64_t timeToEndPrcnt_10ms = TimeToEndPrcnt_ms / 10;
    if (EndPrcnt > 1.0f) {EndPrcnt = 1.0f;}
    if (EndPrcnt < 0.0f) {EndPrcnt = 0.0f;}
    if (timeToEndPrcnt_10ms > 8640000) {timeToEndPrcnt_10ms = 8640000;}
    g_dutyCycleConfig[OutIndx].endVal = (int64_t) (EndPrcnt * 1000000000.0f);
    g_dutyCycleConfig[OutIndx].delta = (((int64_t)g_dutyCycleConfig[OutIndx].endVal - (int64_t)g_dutyCycleConfig[OutIndx].outVal) / (int64_t)timeToEndPrcnt_10ms);   // 10^9 bigger
}

void calc_new_table(triggerTableType* PreparingTablePtr, float PeriodRatio)
{
    triggerTableType emptyData =
    {
        .deltaTimeToNext_us = 0,
        .triggerTime_us = 0,
        .mask = 0
    };
    // clear data
    {
        for (uint8_t i = 0; i < 11; i++)
        {
            PreparingTablePtr[i] = emptyData;
        }
    }


    // create masks for each output
    {
        g_lastTriggerTime_us = 0;
        PeriodRatio = (PeriodRatio >= 1.1f) ? 1.1 : (((PeriodRatio <= 0.9f)) ? 0.9f : PeriodRatio);

        for (uint8_t i = 1; i < g_numOfTriggers; i++)
        {
            uint16_t dutyCyclIndx = (uint16_t) (1000.0 * g_dutyCycles[i-1]);
            uint16_t triggerTime_us = ((float) g_triggerDelayLookupTable_us[dutyCyclIndx]) * PeriodRatio;
            if (triggerTime_us > 200)
            {
                if (0.98f > g_dutyCycles[i-1]) // turn off only if duty cycle is under
                {
                    PreparingTablePtr[i].mask = 0x01 << (i-1);
                }
                PreparingTablePtr[0].mask |= 0x01 << (i-1);
            }
            PreparingTablePtr[i].triggerTime_us = triggerTime_us;
            g_lastTriggerTime_us = (triggerTime_us > g_lastTriggerTime_us) ? triggerTime_us : g_lastTriggerTime_us;
        }
    }

    // sort the table
    {
        std::sort(&PreparingTablePtr[0], &PreparingTablePtr[10], [](triggerTableType &a, triggerTableType &b){return a.triggerTime_us < b.triggerTime_us;});
    }

    // compress the table
    {
        uint8_t compressed = 0; // this one is being compressed into
        uint8_t observed;   // this one is being observed if it is to be compressed or if it is unique

        for (observed = 1; observed < g_numOfTriggers; observed++)
        {
            if (PreparingTablePtr[observed].triggerTime_us > 0)
            {
                if (PreparingTablePtr[compressed].triggerTime_us == PreparingTablePtr[observed].triggerTime_us)
                {
                    PreparingTablePtr[compressed].mask &= (~PreparingTablePtr[observed].mask);
                }
                else
                {
                    compressed++;
                    PreparingTablePtr[compressed].triggerTime_us = PreparingTablePtr[observed].triggerTime_us;
                    PreparingTablePtr[compressed].mask = PreparingTablePtr[compressed - 1].mask & (~PreparingTablePtr[observed].mask);
                    PreparingTablePtr[compressed - 1].deltaTimeToNext_us = PreparingTablePtr[compressed].triggerTime_us - PreparingTablePtr[compressed - 1].triggerTime_us;
                }
            }
        }
        PreparingTablePtr[compressed].mask |= 0x8000;   // add flag that this is the final one
    }
}

float* logic_get_duty_cycles(void)
{
    return g_dutyCycles;
}