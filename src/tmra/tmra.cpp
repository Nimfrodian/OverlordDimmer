#include "tmra.h"
#include "rtdb_vars.h"

static bool tmra_s_moduleInit_tB = false;
static uint32_t tmra_nr_moduleId_U32 = 0;

void tmra_init(tTMRA_INITDATA_STR* TmraCfg)
{
    if (true == tmra_s_moduleInit_tB)
    {
        errh_reportError(ERRH_NOTIF, tmra_nr_moduleId_U32, 0, TMRA_API_INIT_U32, ERRH_MODULE_ALREADY_INIT);
    }
    else if (NULL == TmraCfg)
    {
        errh_reportError(ERRH_ERROR_CRITICAL, tmra_nr_moduleId_U32, 0, TMRA_API_INIT_U32, ERRH_POINTER_IS_NULL);
    }
    else
    {

        tmra_nr_moduleId_U32 = TmraCfg->nr_moduleId_U32;
        tmra_s_moduleInit_tB = true;
    }
}

int64_t tmra_ti_us_getCurrentTime_S64(void)
{
    return (int64_t) 0;//esp_timer_get_time();
}

uint32_t tmra_startTimer(tTMRA_TIMERHANDLE_STR* TimerHandlePtr, uint32_t Ti_us_delay)
{
    uint32_t err = gptimer_enable(*TimerHandlePtr);
    if (err) {errh_reportError(ERRH_ERROR_CRITICAL, tmra_nr_moduleId_U32, 0, TMRA_API_START_TIMER_U32, TMRA_ERR_CANNOT_ENABLE_TIMER_U32);};

    gptimer_alarm_config_t alarm_config = {
        .alarm_count = Ti_us_delay,
    };

    err = gptimer_set_alarm_action(*TimerHandlePtr, &alarm_config);
    if (err) {errh_reportError(ERRH_NOTIF, tmra_nr_moduleId_U32, 0, TMRA_API_START_TIMER_U32, TMRA_ERR_CANNOT_SET_ALARM_U32);};
    err = gptimer_start(*TimerHandlePtr);
    if (err) {errh_reportError(ERRH_NOTIF, tmra_nr_moduleId_U32, 0, TMRA_API_START_TIMER_U32, TMRA_ERR_CANNOT_START_TIMER_U32);};
    return err;
}

uint32_t tmra_stopTimer(tTMRA_TIMERHANDLE_STR* TimerHandlePtr)
{
    uint32_t err = 0;
    err = gptimer_disable(*TimerHandlePtr);
    if (err) {errh_reportError(ERRH_NOTIF, tmra_nr_moduleId_U32, 0, TMRA_API_STOP_TIMER_U32, TMRA_ERR_CANNOT_DISABLE_TIMER_U32);};

    err = gptimer_stop(*TimerHandlePtr);
    if (err) {errh_reportError(ERRH_NOTIF, tmra_nr_moduleId_U32, 0, TMRA_API_STOP_TIMER_U32, TMRA_ERR_CANNOT_STOP_TIMER_U32);};

    return err;
}

uint32_t tmra_createTimer(tTMRA_TIMERHANDLE_STR* TimerHandlePtr, bool (*timerFunc)(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data))
{
    uint32_t err = -1;

    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000, // 1MHz, 1 tick=1us
    };
    gptimer_event_callbacks_t cbs = {
        .on_alarm = timerFunc,
    };

    gptimer_new_timer(&timer_config, TimerHandlePtr);
    gptimer_register_event_callbacks(*TimerHandlePtr, &cbs, nullptr);

    if (err)
    {
        errh_reportError(ERRH_ERROR_CRITICAL, tmra_nr_moduleId_U32, err, TMRA_API_CREATE_TIMER_U32, TMRA_ERR_CANNOT_CREATE_TIMER_U32);
    }

    return err;
}