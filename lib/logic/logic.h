#ifndef LOGIC_H
#define LOGIC_H
#include "stdint.h"
#include "math.h"

#define LAST_MASK_FLAG (0x8000)

typedef struct
{
    int64_t delta;          ///< endVal / timeToEndVal_10ms in 10^9
    int64_t outVal;         ///< current output val in 10^9
    int64_t endVal;         ///< current output val in 10^9
    float endDutCycl;       ///< duty cycle target  [0.0f ... 1.0f]
    float currDutCycl;      ///< current duty cycle [0.0f ... 1.0f]
} dutyCycleStructType;

typedef struct
{
    uint16_t deltaTimeToNext_us;    ///< delay relative to previous triggering
    uint16_t triggerTime_us;        ///< delay time in microseconds to trigger mask
    uint32_t mask;                  ///< true for outputs that are to be turned off. MSB signals last mask
} triggerTableType;

typedef struct
{
    uint32_t numOfPhyOut;   ///< number of physical outputs
} logicInitType;

void logic_init(logicInitType* logicCfg);

/**
 * @brief Function prepares time and duty cycle related information
 * @param OutIndx (physical) output index [0 ... numOfOutputs]
 * @param EndPrcnt final percentage to reach, see .endVal
 * @param TimeToEndPrcnt_ms time to reach the final percentage in ms
 * @return (void)
 */
void config_duty_cycle(uint32_t OutIndx, float EndPrcnt, uint64_t TimeToEndPrcnt_ms);


/**
 * @brief Function prepares information for CAN message.
 * @param DataPtr pointer to 8 bytes of data, i.e., CAN payload
 * @param MsgIdPtr pointer to message ID variable
 * @return (void)
 */
void logic_compose(uint8_t* DataPtr, uint32_t* MsgIdPtr);


/**
 * @brief Function calculates and updates triggering table
 * @param PreparingTablePtr pointer to the table to be updated
 * @param PeriodRatio the current period ratio for time compensation
 * @return (void)
 */
void calc_new_table(triggerTableType* PreparingTablePtr, float PeriodRatio);


#endif