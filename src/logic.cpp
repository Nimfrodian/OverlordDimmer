#include "logic.h"

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
    for (uint32_t i = 0; i < g_numOfTriggers; i++)
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
        uint16_t delayTime = 10000 - 10000 * (acosf(-2.0 * prcntReq + 1) / M_PI);
        if (delayTime > MAX_TRIGGER_DELAY)
        {
            delayTime = MAX_TRIGGER_DELAY;
        }
        g_triggerDelayLookupTable_us[i] = delayTime;
    }
}

void config_duty_cycle(uint32_t OutIndx, float EndPrcnt, uint64_t TimeToEndPrcnt_ms)
{
    if (TimeToEndPrcnt_ms < 10) TimeToEndPrcnt_ms = 10;
    uint64_t timeToEndPrcnt_10ms = TimeToEndPrcnt_ms / 10;
    if (EndPrcnt > 1.0f) EndPrcnt = 1.0f;
    if (EndPrcnt < 0.0f) EndPrcnt = 0.0f;
    if (timeToEndPrcnt_10ms > 8640000) timeToEndPrcnt_10ms = 8640000;
    g_dutyCycleConfig[OutIndx].endVal = (uint64_t) (EndPrcnt * 1000000000.0f);
    g_dutyCycleConfig[OutIndx].delta = (((int64_t)g_dutyCycleConfig[OutIndx].endVal - (int64_t)g_dutyCycleConfig[OutIndx].outVal) / (int64_t)timeToEndPrcnt_10ms);   // 10^9 bigger
}

void calc_new_table(triggerTableType* preparingTable)
{
    for (uint32_t i = 0; i < g_numOfTriggers; i++)
    {
        int dutyCycleIndx = (1000.0 * g_dutyCycles[i]);
        preparingTable[i].delay_us = g_triggerDelayLookupTable_us[dutyCycleIndx];
        if (preparingTable[i].delay_us < MAX_TRIGGER_DELAY)
        {
            preparingTable[i].mask = (0x01UL << i);
        }
        else
        {
            preparingTable[i].mask = 0;
        }
    }

    // sort the table
    for (uint32_t i = 0; i < g_numOfTriggers - 1; i++)
    {
        for (uint32_t j = 0; j < g_numOfTriggers - i - 1; j++)
        {
            if (preparingTable[j].delay_us > preparingTable[j+1].delay_us)
            {
                triggerTableType temp = preparingTable[j];
                preparingTable[j] = preparingTable[j + 1];
                preparingTable[j + 1] = temp;
            }
        }
    }

    // compress the table
    uint32_t i = 0;
    uint32_t numOfTriggers = g_numOfTriggers;
    while (i < numOfTriggers - 1)
    {
        if (preparingTable[i].delay_us == preparingTable[i + 1].delay_us)
        {
            preparingTable[i].mask |= preparingTable[i + 1].mask;
            for (uint32_t j = i + 1; j < numOfTriggers - 1; j++)
            {
                preparingTable[j] = preparingTable[j + 1];
            }
            numOfTriggers--;
        }
        else
        {
            i++;
        }
    }


    // calculate delta times
    preparingTable[0].delta_delay_us = preparingTable[0].delay_us;
    for (int k = g_numOfTriggers-1; k > 0; k--)
    {
        preparingTable[k].delta_delay_us = preparingTable[k].delay_us - preparingTable[k-1].delay_us;
    }
}

float* logic_get_duty_cycles(void)
{
    return g_dutyCycles;
}