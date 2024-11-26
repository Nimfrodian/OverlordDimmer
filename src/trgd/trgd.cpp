#include "trgd.h"
#include "rtdb.h"

static bool trgd_s_moduleInit_tB = false;
static uint32_t trgd_nr_moduleId_U32 = 0;

static bool trgd_fl_updateTable_tB = true;                  ///< flags if a new duty cycle table needs to be calculated. Should occur about once very 10ms
static int64_t trgd_ti_us_prevPeriodStart_S64 = 0;          ///< Time when previous period started in us
static int64_t trgd_ti_us_currPeriodStart_S64 = 10000;      ///< Time when period started in us. Used to calculate period ratio (time deviation)
static uint32_t trgd_nr_numOfTriggeringPins_U32 = 0;        ///< number of physical triggering pins
static uint32_t trgd_ti_us_localZeroCrossTriggerDelay_U32 = 70;  ///< time delay after zero-cross is detected before activating mask

static uint32_t                     trgd_x_triggerCounter_U32 = 0;    ///< variable counts which index of the active table should be applied
static tLGIC_TRIGGERTABLEDATA_STR   trgd_x_triggerTable_astr [2][32]; ///< two trigger tables - one active, one in preparation. 1 initial state and 1 state for each output
static tLGIC_TRIGGERTABLEDATA_STR*  trgd_x_activeTable_pstr    = &trgd_x_triggerTable_astr[1][0]; ///< active table currently being applied
static tLGIC_TRIGGERTABLEDATA_STR*  trgd_x_preparingTable_pstr = &trgd_x_triggerTable_astr[0][0]; ///< table being prepared and will be used in the next cycle

static tTMRA_TIMERHANDLE_STR tmra_h_initialInterrupt_pstr;   ///< initial interrupt triggered by GPIO
static tTMRA_TIMERHANDLE_STR tmra_h_subsequentInterrupt_pstr;   ///< subsequent interrupts triggered by timer. Sets the actual output values

static PINA_nr_GPIO_NUM_E trgd_x_triggerPins_aE[] =
{
    PINA_OUT_NUM_0,
    PINA_OUT_NUM_1,
    PINA_OUT_NUM_2,
    PINA_OUT_NUM_3,
    PINA_OUT_NUM_4,
    PINA_OUT_NUM_5,
    PINA_OUT_NUM_6,
    PINA_OUT_NUM_7,
    PINA_OUT_NUM_8,
    PINA_OUT_NUM_9,
};

/**
 * @brief Helper function applies currently relevant mask to physical outputs
 * @param void
 * @return (void)
 */
static void IRAM_ATTR trgd_applyOutput_ev(void);

void IRAM_ATTR trgd_gpioInterruptHandler_isr(void* arg)
{
    ///< start countdown timer as the zero-cross trigger occurs some time before actual zero-cross
    tmra_startTimer(&tmra_h_initialInterrupt_pstr, trgd_ti_us_localZeroCrossTriggerDelay_U32);
}

void trgd_subsequentTimerInterruptHandler_isr(void* arg)
{
    trgd_applyOutput_ev();
}

void trgd_initialTimerInterruptHandler_isr(void* arg)
{
    trgd_ti_us_prevPeriodStart_S64 = trgd_ti_us_currPeriodStart_S64;
    trgd_ti_us_currPeriodStart_S64 = timh_ti_us_readSystemTime_S64();

    ///< swap tables used
    tLGIC_TRIGGERTABLEDATA_STR* temp_pstr = trgd_x_activeTable_pstr;
    trgd_x_activeTable_pstr = trgd_x_preparingTable_pstr;
    trgd_x_preparingTable_pstr = temp_pstr;


    uint32_t err = tmra_stopTimer(&tmra_h_subsequentInterrupt_pstr);
    if (err)
    {
        errh_reportError(ERRH_WARNING, trgd_nr_moduleId_U32, 0, TRGD_API_INITIAL_INTRPT_HANDLER_U32, TRGD_ERR_CANNOT_STOP_TIMER_U32);
    }

    trgd_x_triggerCounter_U32 = 0;   ///< reset triggering to start from 0
    trgd_applyOutput_ev();

    ///< flag that a new table will be needed
    trgd_fl_updateTable_tB = true;
}

void IRAM_ATTR trgd_applyOutput_ev(void)
{
    uint32_t mask_U32 = trgd_x_activeTable_pstr[trgd_x_triggerCounter_U32].ma_triggerMask_U32;
    pina_setGpioLevel(trgd_x_triggerPins_aE[0], (mask_U32 >> 0) & 0x01);
    pina_setGpioLevel(trgd_x_triggerPins_aE[1], (mask_U32 >> 1) & 0x01);
    pina_setGpioLevel(trgd_x_triggerPins_aE[2], (mask_U32 >> 2) & 0x01);
    pina_setGpioLevel(trgd_x_triggerPins_aE[3], (mask_U32 >> 3) & 0x01);
    pina_setGpioLevel(trgd_x_triggerPins_aE[4], (mask_U32 >> 4) & 0x01);
    pina_setGpioLevel(trgd_x_triggerPins_aE[5], (mask_U32 >> 5) & 0x01);
    pina_setGpioLevel(trgd_x_triggerPins_aE[6], (mask_U32 >> 6) & 0x01);
    pina_setGpioLevel(trgd_x_triggerPins_aE[7], (mask_U32 >> 7) & 0x01);
    pina_setGpioLevel(trgd_x_triggerPins_aE[8], (mask_U32 >> 8) & 0x01);
    pina_setGpioLevel(trgd_x_triggerPins_aE[9], (mask_U32 >> 9) & 0x01);

    ///< reloads timer if this isn't the last triggering
    if (trgd_x_activeTable_pstr[trgd_x_triggerCounter_U32].ma_triggerMask_U32 & LGIC_MA_LAST_MASK_TO_BE_USED_FLAG_U32)   ///< MSB signals that this is the last triggering
    {
        ///< reset the timer when the last operation was completed
        trgd_x_triggerCounter_U32 = 0;
    }
    else
    {
        uint32_t ti_us_delta_U32 = trgd_x_activeTable_pstr[trgd_x_triggerCounter_U32].ti_us_deltaToNext_U16;
        uint32_t err = tmra_startTimer(&tmra_h_subsequentInterrupt_pstr, ti_us_delta_U32);
        if (0 != err)
        {
            errh_reportError(ERRH_WARNING, trgd_nr_moduleId_U32, err, TRGD_API_APPLY_OUTPUT_U32, TRGD_ERR_CANNOT_START_TIMER);
        }
        trgd_x_triggerCounter_U32++;
    }
}


void trgd_init(tTRGD_INITDATA_STR* TrgdCfg)
{
    if (true == trgd_s_moduleInit_tB)
    {
        errh_reportError(ERRH_NOTIF, trgd_nr_moduleId_U32, 0, TRGD_API_INIT_U32, ERRH_MODULE_ALREADY_INIT);
    }
    else if (NULL == TrgdCfg)
    {
        errh_reportError(ERRH_ERROR_CRITICAL, trgd_nr_moduleId_U32, 0, TRGD_API_INIT_U32, ERRH_POINTER_IS_NULL);
    }
    else
    {
        ///< logic init
        trgd_nr_numOfTriggeringPins_U32 = sizeof(trgd_x_triggerPins_aE) / sizeof(trgd_x_triggerPins_aE[0]);
        tLGIC_INITDATA_STR logicCfg
        {
            .nr_moduleId_U32 = MODULE_LGIC,
            .nr_numOfPhysicalOutputs_32 = trgd_nr_numOfTriggeringPins_U32,
        };
        lgic_init(&logicCfg);

        tTIMH_INITDATA_STR timh_cfgData_str =
        {
            .nr_moduleId_U32 = MODULE_TIMH,
            .timh_ti_us_sysTimeFunc_pfS64 = &tmra_ti_us_getCurrentTime_S64,
        };
        timh_init(&timh_cfgData_str);

        ///< prepare triggering table
        tLGIC_TRIGGERTABLEDATA_STR lgic_emptyTriggerData_str =
        {
            .ti_us_deltaToNext_U16 = 0,
            .ti_us_triggerDelay_U16 = 0,
            .ma_triggerMask_U32 = LGIC_MA_LAST_MASK_TO_BE_USED_FLAG_U32,
        };
        uint8_t numOfTables_U08 = sizeof(trgd_x_triggerTable_astr) / sizeof(trgd_x_triggerTable_astr[0]);
        uint8_t numOfTableIndxs_U08 = sizeof(trgd_x_triggerTable_astr[0]) / sizeof(trgd_x_triggerTable_astr[0][0]);
        for (uint8_t table = 0; table < numOfTables_U08; table++)
        {
            for (uint8_t tableIndx = 0; tableIndx < numOfTableIndxs_U08; tableIndx++)
            {
                trgd_x_triggerTable_astr[table][tableIndx] = lgic_emptyTriggerData_str;
            }
        }
        ///< initialize interrupts
        {
            tmra_createTimer(&tmra_h_subsequentInterrupt_pstr, trgd_subsequentTimerInterruptHandler_isr);
            tmra_createTimer(&tmra_h_initialInterrupt_pstr, trgd_initialTimerInterruptHandler_isr);
        }

        {
            pina_setGpioLevel(trgd_x_triggerPins_aE[0], 0);
            pina_setGpioLevel(trgd_x_triggerPins_aE[1], 0);
            pina_setGpioLevel(trgd_x_triggerPins_aE[2], 0);
            pina_setGpioLevel(trgd_x_triggerPins_aE[3], 0);
            pina_setGpioLevel(trgd_x_triggerPins_aE[4], 0);
            pina_setGpioLevel(trgd_x_triggerPins_aE[5], 0);
            pina_setGpioLevel(trgd_x_triggerPins_aE[6], 0);
            pina_setGpioLevel(trgd_x_triggerPins_aE[7], 0);
            pina_setGpioLevel(trgd_x_triggerPins_aE[8], 0);
            pina_setGpioLevel(trgd_x_triggerPins_aE[9], 0);

            for (int i = 0; i < trgd_nr_numOfTriggeringPins_U32; i++)
            {
                pina_setGpioAsOutput(trgd_x_triggerPins_aE[i]);
            }

            pina_setGpioAsInput(PINA_IN_NUM_0);

            pina_setInterruptService(PINA_IN_NUM_0, trgd_gpioInterruptHandler_isr);
        }

        trgd_nr_moduleId_U32 = TrgdCfg->nr_moduleId_U32;
        trgd_s_moduleInit_tB = true;
    }
}

void trgd_run(void)
{
    if (true == trgd_fl_updateTable_tB)
    {
        lgic_calcDutyCycle_10ms();
        float periodRatio_F32 = ((float) (trgd_ti_us_currPeriodStart_S64 - trgd_ti_us_prevPeriodStart_S64)) / 10000.0f;
        lgic_calcNewTable_ev(trgd_x_preparingTable_pstr, periodRatio_F32);
        trgd_fl_updateTable_tB = false;
        trgd_ti_us_localZeroCrossTriggerDelay_U32 = rtdb_read_tU32S(TRGD_TI_US_ZEROCROSSTRIGGERDELAY_U32);
    }
}