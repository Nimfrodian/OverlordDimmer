#ifndef LOGIC_H
#define LOGIC_H
#include "stdint.h"
#include "math.h"

#define LGIC_MA_LAST_MASK_TO_BE_USED_FLAG_U32 ((uint32_t) 0x8000)

typedef struct
{
    int64_t ddc_109p10ms_delta_S64;     ///< dc_109pr_endVal_S64 / timeTodc_109pr_endVal_S64_10ms in 10^9
    int64_t dc_109pr_outVal_S64;        ///< current output val in 10^9
    int64_t dc_109pr_endVal_S64;        ///< current output val in 10^9
    float dc_pr_currVal_F32;            ///< current duty cycle [0.0f ... 1.0f]
} tLGIC_DC_CONFIGDATA_STR;

typedef struct
{
    uint16_t ti_us_deltaToNext_U16;    ///< delay relative to previous triggering
    uint16_t ti_us_triggerDelay_U16;   ///< delay time in microseconds to trigger mask
    uint32_t ma_triggerMask_U32;       ///< true for outputs that are to be turned off. MSB signals last mask
} tLGIC_TRIGGERTABLEDATA_STR;

typedef struct
{
    uint32_t nr_numOfPhysicalOutputs;   ///< number of physical outputs
} tLGIC_INITDATA_STR;

void lgic_init(tLGIC_INITDATA_STR* logicCfg);

/**
 * @brief Function prepares time and duty cycle related information
 * @param OutIndx (physical) output index [0 ... numOfOutputs]
 * @param EndPrcnt final percentage to reach, see .dc_109pr_endVal_S64
 * @param TimeToEndPrcnt_ms time to reach the final percentage in ms
 * @return (void)
 */
void lgic_setDutyCycle_ev(uint32_t OutIndx, float EndPrcnt, uint64_t TimeToEndPrcnt_ms);

/**
 * @brief Function updates duty cycles and should be called periodically
 * @param void
 * @return (void)
 */
void lgic_calcDutyCycle_10ms(void);

/**
 * @brief Function prepares information for CAN message.
 * @param DataPtr pointer to 8 bytes of data, i.e., CAN payload
 * @param MsgIdPtr pointer to message ID variable
 * @return (void)
 */
void lgic_canMsgCompose_100ms(uint8_t* DataPtr, uint32_t* MsgIdPtr);


/**
 * @brief Function calculates and updates triggering table
 * @param PreparingTablePtr pointer to the table to be updated
 * @param PeriodRatio the current period ratio for time compensation
 * @return (void)
 */
void lgic_calcNewTable_ev(tLGIC_TRIGGERTABLEDATA_STR* PreparingTablePtr, float PeriodRatio);


#endif