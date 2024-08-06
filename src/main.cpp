#include "espa.h"
#include "main.h"

static bool main_fl_updateTable_tB = true;                  ///< flags if a new duty cycle table needs to be calculated. Should occur about once very 10ms
static int64_t main_ti_us_prevPeriodStart_S64 = 0;          ///< Time when previous period started in us
static int64_t main_ti_us_currPeriodStart_S64 = 10000;      ///< Time when period started in us. Used to calculate period ratio (time deviation)
static uint32_t main_nr_numOfTriggeringPins_U32 = 0;        ///< number of physical triggering pins
static uint32_t main_ti_us_zeroCrossTriggerDelay_U32 = 70;  ///< time delay after zero-cross is detected before activating mask

static uint32_t                     main_x_triggerCounter_U32 = 0;    ///< variable counts which index of the active table should be applied
static tLGIC_TRIGGERTABLEDATA_STR   main_x_triggerTable_astr [2][32]; ///< two trigger tables - one active, one in preparation. 1 initial state and 1 state for each output
static tLGIC_TRIGGERTABLEDATA_STR*  main_x_activeTable_pstr    = &main_x_triggerTable_astr[1][0]; ///< active table currently being applied
static tLGIC_TRIGGERTABLEDATA_STR*  main_x_preparingTable_pstr = &main_x_triggerTable_astr[0][0]; ///< table being prepared and will be used in the next cycle

static tTMRA_TIMERHANDLE_STR tmra_h_initialInterrupt_pstr;   ///< initial interrupt triggered by GPIO
static tTMRA_TIMERHANDLE_STR tmra_h_subsequentInterrupt_pstr;   ///< subsequent interrupts triggered by timer. Sets the actual output values

static PINA_nr_GPIO_NUM_E main_x_triggerPins_aE[] =
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
static void IRAM_ATTR main_applyOutput_ev(void);

void IRAM_ATTR main_gpioInterruptHandler_isr(void* arg)
{
    ///< start countdown timer as the zero-cross trigger occurs some time before actual zero-cross
    tmra_startTimer(&tmra_h_initialInterrupt_pstr, main_ti_us_zeroCrossTriggerDelay_U32);
}

void main_subsequentTimerInterruptHandler_isr(void* arg)
{
    main_applyOutput_ev();
}

void main_initialTimerInterruptHandler_isr(void* arg)
{
    main_ti_us_prevPeriodStart_S64 = main_ti_us_currPeriodStart_S64;
    main_ti_us_currPeriodStart_S64 = tmra_ti_us_getCurrentTime();

    ///< swap tables used
    tLGIC_TRIGGERTABLEDATA_STR* temp_pstr = main_x_activeTable_pstr;
    main_x_activeTable_pstr = main_x_preparingTable_pstr;
    main_x_preparingTable_pstr = temp_pstr;

    ///< stop active timer if it is still active
    if (tmra_isTimerActive(&tmra_h_subsequentInterrupt_pstr))
    {
        uint32_t err = tmra_stopTimer(&tmra_h_subsequentInterrupt_pstr);
        if (0 != err)
        {
            // TODO: Report error
        }
    }

    main_x_triggerCounter_U32 = 0;   ///< reset triggering to start from 0
    main_applyOutput_ev();

    ///< flag that a new table will be needed
    main_fl_updateTable_tB = true;
}

void IRAM_ATTR main_applyOutput_ev(void)
{
    uint32_t mask_U32 = main_x_activeTable_pstr[main_x_triggerCounter_U32].ma_triggerMask_U32;
    pina_setGpioLevel(main_x_triggerPins_aE[0], (mask_U32 >> 0) & 0x01);
    pina_setGpioLevel(main_x_triggerPins_aE[1], (mask_U32 >> 1) & 0x01);
    pina_setGpioLevel(main_x_triggerPins_aE[2], (mask_U32 >> 2) & 0x01);
    pina_setGpioLevel(main_x_triggerPins_aE[3], (mask_U32 >> 3) & 0x01);
    pina_setGpioLevel(main_x_triggerPins_aE[4], (mask_U32 >> 4) & 0x01);
    pina_setGpioLevel(main_x_triggerPins_aE[5], (mask_U32 >> 5) & 0x01);
    pina_setGpioLevel(main_x_triggerPins_aE[6], (mask_U32 >> 6) & 0x01);
    pina_setGpioLevel(main_x_triggerPins_aE[7], (mask_U32 >> 7) & 0x01);
    pina_setGpioLevel(main_x_triggerPins_aE[8], (mask_U32 >> 8) & 0x01);
    pina_setGpioLevel(main_x_triggerPins_aE[9], (mask_U32 >> 9) & 0x01);

    ///< reloads timer if this isn't the last triggering
    if (main_x_activeTable_pstr[main_x_triggerCounter_U32].ma_triggerMask_U32 & LGIC_MA_LAST_MASK_TO_BE_USED_FLAG_U32)   ///< MSB signals that this is the last triggering
    {
        ///< reset the timer when the last operation was completed
        main_x_triggerCounter_U32 = 0;
    }
    else
    {
        uint32_t ti_us_delta_U32 = main_x_activeTable_pstr[main_x_triggerCounter_U32].ti_us_deltaToNext_U16;
        uint32_t err = tmra_startTimer(&tmra_h_subsequentInterrupt_pstr, ti_us_delta_U32);
        if (0 != err)
        {
            //TODO: Log error ("Main", "Failed to start tmra_h_subsequentInterrupt_pstr with error code %u", err);
        }
        main_x_triggerCounter_U32++;
    }
}

extern "C" void app_main()
{
    // initialize serial communication
    {
        sera_init();
    }

    {
        ///< logic init
        main_nr_numOfTriggeringPins_U32 = sizeof(main_x_triggerPins_aE) / sizeof(main_x_triggerPins_aE[0]);
        tLGIC_INITDATA_STR logicCfg
        {
            .nr_numOfPhysicalOutputs = main_nr_numOfTriggeringPins_U32,
        };
        lgic_init(&logicCfg);

        ///< prepare triggering table
        tLGIC_TRIGGERTABLEDATA_STR lgic_emptyTriggerData_str =
        {
            .ti_us_deltaToNext_U16 = 0,
            .ti_us_triggerDelay_U16 = 0,
            .ma_triggerMask_U32 = LGIC_MA_LAST_MASK_TO_BE_USED_FLAG_U32,
        };
        uint8_t numOfTables_U08 = sizeof(main_x_triggerTable_astr) / sizeof(main_x_triggerTable_astr[0]);
        uint8_t numOfTableIndxs_U08 = sizeof(main_x_triggerTable_astr[0]) / sizeof(main_x_triggerTable_astr[0][0]);
        for (uint8_t table = 0; table < numOfTables_U08; table++)
        {
            for (uint8_t tableIndx = 0; tableIndx < numOfTableIndxs_U08; tableIndx++)
            {
                main_x_triggerTable_astr[table][tableIndx] = lgic_emptyTriggerData_str;
            }
        }
    }

    ///< initialize interrupts
    {
        tmra_createTimer(&tmra_h_subsequentInterrupt_pstr, main_subsequentTimerInterruptHandler_isr);
        tmra_createTimer(&tmra_h_initialInterrupt_pstr, main_initialTimerInterruptHandler_isr);
    }

    {
        pina_setGpioLevel(main_x_triggerPins_aE[0], 0);
        pina_setGpioLevel(main_x_triggerPins_aE[1], 0);
        pina_setGpioLevel(main_x_triggerPins_aE[2], 0);
        pina_setGpioLevel(main_x_triggerPins_aE[3], 0);
        pina_setGpioLevel(main_x_triggerPins_aE[4], 0);
        pina_setGpioLevel(main_x_triggerPins_aE[5], 0);
        pina_setGpioLevel(main_x_triggerPins_aE[6], 0);
        pina_setGpioLevel(main_x_triggerPins_aE[7], 0);
        pina_setGpioLevel(main_x_triggerPins_aE[8], 0);
        pina_setGpioLevel(main_x_triggerPins_aE[9], 0);

        for (int i = 0; i < main_nr_numOfTriggeringPins_U32; i++)
        {
            pina_setGpioAsOutput(main_x_triggerPins_aE[i]);
        }

        pina_setGpioAsInput(PINA_IN_NUM_0);

        pina_setInterruptService(PINA_IN_NUM_0, main_gpioInterruptHandler_isr);
    }

    ComCan_init();

    while(true)
    {
        if (true == main_fl_updateTable_tB)
        {
            lgic_calcDutyCycle_10ms();
            float periodRatio_F32 = ((float) (main_ti_us_currPeriodStart_S64 - main_ti_us_prevPeriodStart_S64)) / 10000.0f;
            lgic_calcNewTable_ev(main_x_preparingTable_pstr, periodRatio_F32);
            main_fl_updateTable_tB = false;
        }

        ComCan_receive();
        canRxDataType* rxDataPtr = ComCan_get_rxData();
        if (1 == rxDataPtr->dataReceived)
        {
            for (uint32_t i_U32 = 0; i_U32 < main_nr_numOfTriggeringPins_U32; i_U32++)
            {
                if ((rxDataPtr->triggerIndexMask >> i_U32) & 0x01)
                {
                    float pr_endVal_F32 = ((float) rxDataPtr->dutyCycleReq) / 1000.0f;
                    pr_endVal_F32 = (pr_endVal_F32 > 1.0f)? 1.0f : (pr_endVal_F32 < 0.0f) ? 0.0f : pr_endVal_F32;
                    lgic_setDutyCycle_ev(i_U32, pr_endVal_F32, rxDataPtr->timeRequest_10ms);
                }
            }
            rxDataPtr->dataReceived = 0;
        }

        static uint32_t counter = 0;
        if (counter > 100)
        {
            CAN_frame_t canMsg
            {
                .FIR =
                {
                    .B =
                    {
                        .DLC = 8,
                        .unknown_2 = 0,
                        .RTR = CAN_no_RTR,
                        .FF = CAN_frame_std,
                        .reserved_24 = 0,
                    }
                },
                .MsgID = 0x7FF,
                .data = {.u8 = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}
            };

            lgic_canMsgCompose_100ms(&canMsg.data.u8[0], &canMsg.MsgID);
            ComCan_transmit(&canMsg, 1);
            counter = 0;
        }
        counter += 5;

        vTaskDelay(5 / portTICK_PERIOD_MS);
    }
}