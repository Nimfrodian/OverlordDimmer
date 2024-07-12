#ifndef LOGIC_H
#define LOGIC_H
#include "stdint.h"
#include "math.h"

#define MAX_TRIGGER_DELAY (9900U)
#define MIN_TRIGGER_DELAY   (0U)

typedef struct
{
    uint32_t numOfTriggers;
} logicConfigType;

typedef struct
{
    int64_t delta;         ///< endVal / timeToEndVal_10ms in 10^9
    int64_t outVal;        ///< current output val in 10^9
    int64_t endVal;        ///< current output val in 10^9
    float endDutCycl;      ///< duty cycle target
} dutyCycleConfigType;

typedef struct
{
    uint16_t deltaDelay_us;     ///< delay relative to previous triggering
    uint16_t delay_us;          ///< delay time in microseconds to trigger mask
    uint32_t mask;              ///< true for outputs that are to be turned off
} triggerTableType;

extern void init_logic(logicConfigType const* logicCfgPtr);
extern void calc_duty_cycle(void);
extern void initLookupTable();
extern void config_duty_cycle(uint32_t OutIndx, float EndPrcnt, uint64_t TimeToEndPrcnt_ms);
extern void calc_new_table(triggerTableType* preparingTable);

extern float* logic_get_duty_cycles(void);
extern uint32_t logic_get_logicActiveMask(void);

#endif