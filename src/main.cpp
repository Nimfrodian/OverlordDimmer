#include "main.h"

void main_run(void)
{
    tCANM_X_CANMSGDATA_STR* rxDataPtr = canm_pstr_readCanMsgData(CAN_COMMAND_MESSAGE);
    if (1 == rxDataPtr->canRdyForParse_tB)
    {
        lgic_canMsgParse_ev( rxDataPtr->canMsg_str.data, &rxDataPtr->canMsg_str.identifier);
    }

    static uint32_t counter = 0;
    if (counter > 100)
    {
        tCANM_X_CANMSGDATA_STR* msgPtr0_pstr = canm_pstr_readCanMsgData(CAN_REPLY_MESSAGE0);
        tCANM_X_CANMSGDATA_STR* msgPtr1_pstr = canm_pstr_readCanMsgData(CAN_REPLY_MESSAGE1);
        msgPtr0_pstr->canMsg_str.identifier = 0x96;
        msgPtr1_pstr->canMsg_str.identifier = 0x97;

        lgic_canMsgCompose_100ms(msgPtr0_pstr->canMsg_str.data, &msgPtr0_pstr->canMsg_str.identifier);
        lgic_canMsgCompose_100ms(msgPtr1_pstr->canMsg_str.data, &msgPtr1_pstr->canMsg_str.identifier);

        canm_x_flagCanMsgForTx(CAN_REPLY_MESSAGE0);
        canm_x_flagCanMsgForTx(CAN_REPLY_MESSAGE1);
        counter = 0;
    }
    counter += MAIN_TI_ms_TASK_DELAY_U32;
}

extern "C" void app_main()
{
    {
        tSERA_INITDATA_STR SeraCfg =
        {
            .nr_moduleId_U32 = MODULE_SERA,
        };
        sera_init(&SeraCfg);
        sera_print("Sera module initialized\n");

        tTRGD_INITDATA_STR TrgdCfg =
        {
            .nr_moduleId_U32 = MODULE_TRGD,
        };
        trgd_init(&TrgdCfg);
        sera_print("Trgd module initialized\n");

        tCANM_INITDATA_STR CanmCfg =
        {
            .nr_moduleId_U32 = MODULE_CANM,
        };
        canm_init(&CanmCfg);
        sera_print("Canm module initialized\n");

        sera_print("Initialization time: %lli us\n", timh_ti_us_readSystemTime_S64());
    }

    while(true)
    {
        main_run();

        canm_transceive_run();

        trgd_run();

        vTaskDelay(MAIN_TI_ms_TASK_DELAY_U32 / portTICK_PERIOD_MS);
    }
}