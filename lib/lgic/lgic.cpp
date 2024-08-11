#include "lgic.h"
#include <algorithm>

#define LGIC_NR_TRIGGER_DELAY_TABLE_SIZE_U16 ((uint16_t) 1001)  ///< size of the triggering delay lookup table
#define LGIC_TI_us_MIN_TRIGGER_DELAY_U16 ((uint16_t) 100)      ///< maximum trigger delay to avoid delaying it into the next half-period
#define LGIC_TI_us_MAX_TRIGGER_DELAY_U16 ((uint16_t) 9900)      ///< maximum trigger delay to avoid delaying it into the next half-period
#define LGIC_TI_us_HALF_PERIOD_TIME_U16 ((uint16_t) 10000)      ///< for 50Hz system the period is 20ms, half is 10ms or 10000us

// GLOBAL VARIABLES //////////////////////////////////////////

static bool lgic_s_moduleInit_tB = false;
static uint32_t lgic_nr_numOfTriggeringPins_U32 = 0; ///< number of physical triggering pins
static tLGIC_DC_CONFIGDATA_STR lgic_dc_config_astr[32];
static uint16_t lgic_ti_us_triggerDelayLookupTable_U16[LGIC_NR_TRIGGER_DELAY_TABLE_SIZE_U16]; ///< given a duty cycle in 0.1% resolution (i.e., 100 is 10%) returns time it takes to turn off output to reach desired duty cycle

//////////////////////////////////////////////////////////////
// FUNCTIONS /////////////////////////////////////////////////
void lgic_init(tLGIC_INITDATA_STR* LogicCfg)
{
    if (NULL == LogicCfg)
    {
        // TODO: report ERROR
    }
    else
    {
        // calculate the delay lookup table
        lgic_nr_numOfTriggeringPins_U32 = LogicCfg->nr_numOfPhysicalOutputs;
        for (uint32_t i_U32 = 0; i_U32 < LGIC_NR_TRIGGER_DELAY_TABLE_SIZE_U16; i_U32++)
        {
            float lgic_pr_request_F32 = ((float) i_U32) / ((float)(LGIC_NR_TRIGGER_DELAY_TABLE_SIZE_U16 - 1));
            uint16_t lgic_ti_us_triggerDelay_U16; ///< delay with which triggering must happen to achieve given duty cycle, e.g. 5000us for 50% in 10ms period
            lgic_ti_us_triggerDelay_U16 = LGIC_TI_us_HALF_PERIOD_TIME_U16 * (acosf(-2.0 * lgic_pr_request_F32 + 1) / M_PI);
            if (lgic_ti_us_triggerDelay_U16 > LGIC_TI_us_HALF_PERIOD_TIME_U16)
            {
                lgic_ti_us_triggerDelay_U16 = LGIC_TI_us_HALF_PERIOD_TIME_U16;
            }
            lgic_ti_us_triggerDelayLookupTable_U16[i_U32] = lgic_ti_us_triggerDelay_U16;
        }

        // mark module as initialized
        lgic_s_moduleInit_tB = true;
    }
}

void lgic_setDutyCycle_ev(uint32_t OutIndx, float EndPrcnt, uint64_t TimeToEndPrcnt_ms)
{
    if (!lgic_s_moduleInit_tB)
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
        lgic_dc_config_astr[OutIndx].dc_109pr_endVal_S64 = (int64_t) (EndPrcnt * 1000000000.0f);
        lgic_dc_config_astr[OutIndx].ddc_109p10ms_delta_S64 = (((int64_t)lgic_dc_config_astr[OutIndx].dc_109pr_endVal_S64 - (int64_t)lgic_dc_config_astr[OutIndx].dc_109pr_outVal_S64) / (int64_t)timeToEndPrcnt_10ms);   // 10^9 bigger
    }
}

void lgic_calcDutyCycle_10ms(void)
{
    for (uint32_t i_U32 = 0; i_U32 < (lgic_nr_numOfTriggeringPins_U32); i_U32++)
    {
        int64_t diff_S64 = (lgic_dc_config_astr[i_U32].ddc_109p10ms_delta_S64 + lgic_dc_config_astr[i_U32].dc_109pr_outVal_S64);
        if (lgic_dc_config_astr[i_U32].ddc_109p10ms_delta_S64 > 0)
        {
            if (diff_S64 > lgic_dc_config_astr[i_U32].dc_109pr_endVal_S64)
            {
                lgic_dc_config_astr[i_U32].dc_109pr_outVal_S64 = lgic_dc_config_astr[i_U32].dc_109pr_endVal_S64;
            }
            else
            {
                lgic_dc_config_astr[i_U32].dc_109pr_outVal_S64 += lgic_dc_config_astr[i_U32].ddc_109p10ms_delta_S64;
            }
        }
        else
        {
            if (diff_S64 < lgic_dc_config_astr[i_U32].dc_109pr_endVal_S64)
            {
                lgic_dc_config_astr[i_U32].dc_109pr_outVal_S64 = lgic_dc_config_astr[i_U32].dc_109pr_endVal_S64;
            }
            else
            {
                lgic_dc_config_astr[i_U32].dc_109pr_outVal_S64 += lgic_dc_config_astr[i_U32].ddc_109p10ms_delta_S64;
            }
        }
        float dutyCycle_F32 = ((float) lgic_dc_config_astr[i_U32].dc_109pr_outVal_S64) / 1000000000.0f;
        if (dutyCycle_F32 > 1.0f)
        {
            dutyCycle_F32 = 1.0f;
        }
        else if (dutyCycle_F32 < 0.0f)
        {
            dutyCycle_F32 = 0.0f;
        }
        lgic_dc_config_astr[i_U32].dc_pr_currVal_F32 = dutyCycle_F32;
    }
}

void lgic_canMsgCompose_100ms(uint8_t* DataPtr, uint32_t* MsgIdPtr)
{
    if (!lgic_s_moduleInit_tB)
    {
        // TODO: report ERROR
    }
    else
    {
        uint16_t dc_1000pr_currDutyCycles_U16[32] = {0};
        for (int i = 0; i < lgic_nr_numOfTriggeringPins_U32; i++)
        {
            dc_1000pr_currDutyCycles_U16[i] = uint16_t (lgic_dc_config_astr[i].dc_pr_currVal_F32 * 1000.0f);
        }

        if (0x96 == (*MsgIdPtr))
        {
            // pack lgic_d_currentDutyCycles_F32 into CanMsgPtr->data.u8
            DataPtr[0] = (dc_1000pr_currDutyCycles_U16[0] & 0xFF);
            DataPtr[1] = ((dc_1000pr_currDutyCycles_U16[1] & 0x3F) << 2) | ((dc_1000pr_currDutyCycles_U16[0] >> 8) & 0x03);
            DataPtr[2] = ((dc_1000pr_currDutyCycles_U16[2] & 0x0F) << 4) | ((dc_1000pr_currDutyCycles_U16[1] >> 6) & 0x0F);
            DataPtr[3] = ((dc_1000pr_currDutyCycles_U16[3] & 0x03) << 6) | ((dc_1000pr_currDutyCycles_U16[2] >> 4) & 0x3F);
            DataPtr[4] = ((dc_1000pr_currDutyCycles_U16[3] >> 2) & 0xFF);
            DataPtr[5] = (dc_1000pr_currDutyCycles_U16[4] & 0xFF);
            DataPtr[6] = ((dc_1000pr_currDutyCycles_U16[4] >> 8) & 0x03);
            DataPtr[7] = 0;
        }
        else if (0x97 == (*MsgIdPtr))
        {
            // pack lgic_d_currentDutyCycles_F32 into canMsg0x97.data.u8
            DataPtr[0] = (dc_1000pr_currDutyCycles_U16[5] & 0xFF);
            DataPtr[1] = ((dc_1000pr_currDutyCycles_U16[6] & 0x3F) << 2) | ((dc_1000pr_currDutyCycles_U16[5] >> 8) & 0x03);
            DataPtr[2] = ((dc_1000pr_currDutyCycles_U16[7] & 0x0F) << 4) | ((dc_1000pr_currDutyCycles_U16[6] >> 6) & 0x0F);
            DataPtr[3] = ((dc_1000pr_currDutyCycles_U16[8] & 0x03) << 6) | ((dc_1000pr_currDutyCycles_U16[7] >> 4) & 0x3F);
            DataPtr[4] = ((dc_1000pr_currDutyCycles_U16[8] >> 2) & 0xFF);
            DataPtr[5] = (dc_1000pr_currDutyCycles_U16[9] & 0xFF);
            DataPtr[6] = ((dc_1000pr_currDutyCycles_U16[9] >> 8) & 0x03);
            DataPtr[7] = 0;
        }
        else
        {
            // TODO: report ERROR
        }
    }
}

void lgic_canMsgParse_ev(uint8_t* DataPtr, uint32_t* MsgIdPtr)
{
    if (!lgic_s_moduleInit_tB)
    {
        // TODO: report ERROR
    }
    else
    {
        switch (*MsgIdPtr)
        {
            case (0x95):
            {
                uint32_t timeRequest_10ms_U32 = ((DataPtr[2]& 0xFF) << 16) | ((DataPtr[1] & 0xFF) << 8) | ((DataPtr[0] & 0xFF) << 0);
                uint32_t dutyReq_U32 = ((DataPtr[4] & 0x03) << 8) | ((DataPtr[3] & 0xFF) << 0);
                uint32_t dutyCycleReq_U32 = (dutyReq_U32 > 1000) ? 1000 : dutyReq_U32;
                uint32_t triggerIndexMask_U32 = ((DataPtr[7] & 0xFF) << 16) | ((DataPtr[6] & 0xFF) << 8) | ((DataPtr[5] & 0xFF) << 0);

                for (uint32_t i_U32 = 0; i_U32 < lgic_nr_numOfTriggeringPins_U32; i_U32++)
                {
                    if ((triggerIndexMask_U32 >> i_U32) & 0x01)
                    {
                        float pr_endVal_F32 = ((float) dutyCycleReq_U32) / 1000.0f;
                        pr_endVal_F32 = (pr_endVal_F32 > 1.0f)? 1.0f : (pr_endVal_F32 < 0.0f) ? 0.0f : pr_endVal_F32;
                        lgic_setDutyCycle_ev(i_U32, pr_endVal_F32, timeRequest_10ms_U32);
                    }
                }
            }
            default:
            {
                // TODO: report error
            }
        }

    }
}


void lgic_calcNewTable_ev(tLGIC_TRIGGERTABLEDATA_STR* PreparingTablePtr, float PeriodRatio)
{
    // clean triggering table
    tLGIC_TRIGGERTABLEDATA_STR lgic_emptyTriggeringData_str =
    {
        .ti_us_deltaToNext_U16 = 0,
        .ti_us_triggerDelay_U16 = 0,
        .ma_triggerMask_U32 = 0,
    };
    for (uint8_t i = 0; i < (lgic_nr_numOfTriggeringPins_U32 + 1); i++)
    {
        PreparingTablePtr[i] = lgic_emptyTriggeringData_str;
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
                for (uint8_t i = 0; i < lgic_nr_numOfTriggeringPins_U32; i++)
                {
                    if (0.0f < lgic_dc_config_astr[i].dc_pr_currVal_F32)
                    {
                        PreparingTablePtr[0].ma_triggerMask_U32 |= (0x01 << i);
                    }
                }
            }

            // sets each subsequent mask
            {
                // sets the output that is to be turned off to 0 in a copied initial mask
                // e.g., 0b 0011 0000 0100 if output 0 is to be turned off
                {
                    for (uint8_t i = 0; i < lgic_nr_numOfTriggeringPins_U32; i++)
                    {
                        uint16_t dutyCyclIndx = (uint16_t) (1000.0 * lgic_dc_config_astr[i].dc_pr_currVal_F32);
                        float ti_us_triggerTime_F32 = ((float) lgic_ti_us_triggerDelayLookupTable_U16[dutyCyclIndx]);

                        // limit minimum triggering time
                        if (ti_us_triggerTime_F32 < LGIC_TI_us_MIN_TRIGGER_DELAY_U16)
                        {
                            ti_us_triggerTime_F32 = LGIC_TI_us_MIN_TRIGGER_DELAY_U16;
                        }

                        PreparingTablePtr[i+1].ti_us_triggerDelay_U16 = ((float) ti_us_triggerTime_F32) * PeriodRatio;

                        // turn off only if triggering time is not too close to end of period
                        if (LGIC_TI_us_MAX_TRIGGER_DELAY_U16 > ti_us_triggerTime_F32)
                        {
                            uint32_t ma_turnOffMask_U32 = ~((0x01 << i)); // sets last mask flag to 0 as well as this output
                            PreparingTablePtr[i+1].ma_triggerMask_U32 = PreparingTablePtr[0].ma_triggerMask_U32 & ma_turnOffMask_U32;
                        }
                        else
                        {
                            PreparingTablePtr[i+1].ma_triggerMask_U32 = PreparingTablePtr[0].ma_triggerMask_U32;   // Don't change mask and flag as final
                        }
                    }
                }
            }

            // sorts the table by triggering time
            {
                std::sort(&PreparingTablePtr[0], &PreparingTablePtr[lgic_nr_numOfTriggeringPins_U32 + 1], [](tLGIC_TRIGGERTABLEDATA_STR &a, tLGIC_TRIGGERTABLEDATA_STR &b){return a.ti_us_triggerDelay_U16 < b.ti_us_triggerDelay_U16;});
            }

            // compress the table
            {
                uint8_t ix_compressingInto_U08 = 0; // this one is being compressed into
                uint8_t ix_observed_U08;   // this one is being observed if it is to be compressed or if it is unique

                for (ix_observed_U08 = 1; ix_observed_U08 < (lgic_nr_numOfTriggeringPins_U32 + 1); ix_observed_U08++)
                {
                    if (PreparingTablePtr[ix_observed_U08].ti_us_triggerDelay_U16 > 0) // skip to the first member actually delayed
                    {
                        if (PreparingTablePtr[ix_compressingInto_U08].ti_us_triggerDelay_U16 == PreparingTablePtr[ix_observed_U08].ti_us_triggerDelay_U16)
                        {
                            PreparingTablePtr[ix_compressingInto_U08].ma_triggerMask_U32 &= (PreparingTablePtr[ix_observed_U08].ma_triggerMask_U32);
                        }
                        else
                        {
                            ix_compressingInto_U08++;
                            PreparingTablePtr[ix_compressingInto_U08].ti_us_triggerDelay_U16 = PreparingTablePtr[ix_observed_U08].ti_us_triggerDelay_U16;
                            PreparingTablePtr[ix_compressingInto_U08].ma_triggerMask_U32 = PreparingTablePtr[ix_compressingInto_U08 - 1].ma_triggerMask_U32 & (PreparingTablePtr[ix_observed_U08].ma_triggerMask_U32);
                            PreparingTablePtr[ix_compressingInto_U08 - 1].ti_us_deltaToNext_U16 = PreparingTablePtr[ix_compressingInto_U08].ti_us_triggerDelay_U16 - PreparingTablePtr[ix_compressingInto_U08 - 1].ti_us_triggerDelay_U16;
                            uint16_t maxTriggerTime = LGIC_TI_us_MAX_TRIGGER_DELAY_U16 * PeriodRatio;
                            if (PreparingTablePtr[ix_compressingInto_U08].ti_us_triggerDelay_U16 >= maxTriggerTime)
                            {
                                PreparingTablePtr[ix_compressingInto_U08-1].ma_triggerMask_U32 |= LGIC_MA_LAST_MASK_TO_BE_USED_FLAG_U32;   // if this one is to be triggered after max trigger delay, mark previous as last
                            }
                        }
                    }
                }
                PreparingTablePtr[ix_compressingInto_U08].ma_triggerMask_U32 |= LGIC_MA_LAST_MASK_TO_BE_USED_FLAG_U32;   // flag that the last one to be compressed into is the last one to trigger (possibly redundant)
            }
        }
    }
}