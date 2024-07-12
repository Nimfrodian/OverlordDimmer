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
    triggerTableType internalTable[11];   // internal triggering table to be copied at the end of the algorithm

    triggerTableType emptyData =
    {
        .deltaDelay_us = 0,
        .delay_us = 0,
        .mask = 0
    };
    // clear data
    {
        for (uint8_t i = 0; i < 11; i++)
        {
            internalTable[i] = emptyData;
            preparingTable[i] = emptyData;
        }
    }



    // create masks for each output
    {
        uint32_t runningMask = 0;
        for (uint32_t i = 0; i < g_numOfTriggers; i++)
        { // subsequent states
            internalTable[i+1].mask = (~0x0); // 0xFFF....
            int dutyCycleIndx = (1000.0 * g_dutyCycles[i]);
            internalTable[i+1].delay_us = g_triggerDelayLookupTable_us[dutyCycleIndx];
            if (internalTable[i+1].delay_us != 0)
            {
                runningMask |= 0x01 << (i);
                internalTable[i+1].mask &= ~((0x1) << (i));
            }
        }
        // starting outputs
        internalTable[0].mask = runningMask;
    }

    // sort the table
    {
        for (uint32_t i = 0; i < (g_numOfTriggers); i++)
        {
            for (uint32_t j = 0; j < (g_numOfTriggers - i); j++)
            {
                if (internalTable[j].delay_us > internalTable[j+1].delay_us)
                {
                    triggerTableType temp = internalTable[j];
                    internalTable[j] = internalTable[j + 1];
                    internalTable[j + 1] = temp;
                }
            }
        }
    }

    // compress the table
    {
        uint8_t outputIndx = 0;
        uint32_t outputMask = ~0; // all 1s
        for (int i = 0; i < 10; i++)
        {
            outputMask &= internalTable[i].mask;
            if (internalTable[i].delay_us != internalTable[i+1].delay_us)
            {
                preparingTable[outputIndx].delay_us = internalTable[i].delay_us;
                preparingTable[outputIndx].mask = outputMask;
                outputIndx++;
            }

            // Not all triggers were active and it needs to turn it off "manually"
            if (outputIndx < 10)
            {
                preparingTable[outputIndx].delay_us = internalTable[10].delay_us;
            }
        }
    }

    // calculate delta times
    preparingTable[0].deltaDelay_us = preparingTable[0].delay_us;
    for (int k = g_numOfTriggers-1; k > 0; k--)
    {
        if (preparingTable[k].delay_us != 0)
        {
            preparingTable[k].deltaDelay_us = preparingTable[k].delay_us - preparingTable[k-1].delay_us;
        }
    }
}

float* logic_get_duty_cycles(void)
{
    return g_dutyCycles;
}