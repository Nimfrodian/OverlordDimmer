#include "logic.h"
#include <algorithm>

#define TRIGGER_DELAY_TABLE_SIZE 1001

uint32_t g_numOfTriggers = 0;
dutyCycleConfigType g_dutyCycleConfig[32];
float g_dutyCycles[32] = {0};  // holds duty cycles as calculated by calc_duty_cycle

uint16_t g_triggerDelayLookupTable_us[TRIGGER_DELAY_TABLE_SIZE];

void init_logic(logicConfigType const* logicCfgPtr)
{
    g_numOfTriggers = logicCfgPtr->numOfTriggers;

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
        if (delayTime > MAX_TRIGGER_DELAY)
        {
            delayTime = MAX_TRIGGER_DELAY;
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

void calc_new_table(triggerTableType* preparingTable)
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
            preparingTable[i] = emptyData;
        }
    }


    // create masks for each output
    {
        for (uint8_t i = 1; i < g_numOfTriggers; i++)
        {
            uint16_t dutyCyclIndx = (uint16_t) (1000.0 * g_dutyCycles[i-1]);
            uint16_t triggerTime_us = g_triggerDelayLookupTable_us[dutyCyclIndx];
            if (triggerTime_us > 0)
            {
                if (triggerTime_us < MAX_TRIGGER_DELAY) // turn off only if trigger time is before next interval
                {
                    preparingTable[i].mask = 0x01 << (i-1);
                }
                preparingTable[0].mask |= 0x01 << (i-1);
            }
            preparingTable[i].triggerTime_us = triggerTime_us;
        }
    }

    // sort the table
    {
        std::sort(&preparingTable[0], &preparingTable[10], [](triggerTableType &a, triggerTableType &b){return a.triggerTime_us < b.triggerTime_us;});
    }

    // compress the table
     {
        uint8_t compressed = 0; // this one is being compressed into
        uint8_t observed;   // this one is being observed if it is to be compressed or if it is unique

        for (observed = 1; observed < g_numOfTriggers; observed++)
        {
            if (preparingTable[observed].triggerTime_us > 0)
            {
                if (preparingTable[compressed].triggerTime_us == preparingTable[observed].triggerTime_us)
                {
                    preparingTable[compressed].mask &= (~preparingTable[observed].mask);
                }
                else
                {
                    compressed++;
                    preparingTable[compressed].triggerTime_us = preparingTable[observed].triggerTime_us;
                    preparingTable[compressed].mask = preparingTable[compressed - 1].mask & (~preparingTable[observed].mask);
                    preparingTable[compressed - 1].deltaTimeToNext_us = preparingTable[compressed].triggerTime_us - preparingTable[compressed - 1].triggerTime_us;
                }
            }
        }
    }
}

float* logic_get_duty_cycles(void)
{
    return g_dutyCycles;
}