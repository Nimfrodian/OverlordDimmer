#include "tmra.h"

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
    return (int64_t) esp_timer_get_time();
}

uint32_t tmra_startTimer(tTMRA_TIMERHANDLE_STR* TimerHandlePtr, uint32_t Ti_us_delay)
{
    return (uint32_t) esp_timer_start_once(*TimerHandlePtr, Ti_us_delay);
}

uint32_t tmra_isTimerActive(tTMRA_TIMERHANDLE_STR* TimerHandlePtr)
{
    return (uint32_t) esp_timer_is_active(*TimerHandlePtr);
}

uint32_t tmra_stopTimer(tTMRA_TIMERHANDLE_STR* TimerHandlePtr)
{
    uint32_t err = 0;
    if (tmra_isTimerActive(TimerHandlePtr))
    {
        err = (uint32_t) esp_timer_stop(*TimerHandlePtr);
    }
    return err;
}

uint32_t tmra_createTimer(tTMRA_TIMERHANDLE_STR* TimerHandlePtr, void (*timerFunc)(void* arg))
{
    uint32_t err = -1;
    esp_timer_create_args_t* const timerArgs_pstr = (esp_timer_create_args_t*) malloc(sizeof(esp_timer_create_args_t));

    if (NULL == timerArgs_pstr)
    {
        errh_reportError(ERRH_ERROR_CRITICAL, tmra_nr_moduleId_U32, 0, TMRA_API_CREATE_TIMER_U32, TMRA_ERR_MALLOC_RETURNED_NULL_U32);
    }
    else
    {
        timerArgs_pstr->callback = timerFunc;
        timerArgs_pstr->arg = NULL;
        timerArgs_pstr->dispatch_method = ESP_TIMER_TASK;
        timerArgs_pstr->name = "";
        timerArgs_pstr->skip_unhandled_events = true;
        err = (uint32_t) esp_timer_create(timerArgs_pstr, TimerHandlePtr);

        if (err)
        {
            errh_reportError(ERRH_ERROR_CRITICAL, tmra_nr_moduleId_U32, err, TMRA_API_CREATE_TIMER_U32, TMRA_ERR_CANNOT_CREATE_TIMER_U32);
        }
    }
    return err;
}