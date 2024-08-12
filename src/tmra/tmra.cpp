#include "tmra.h"

static bool tmra_s_moduleInit_tB = false;
static uint32_t tmra_nr_moduleId_U32 = 0;

void tmra_init(tTMRA_INITDATA_STR* TmraCfg)
{
    if (true == tmra_s_moduleInit_tB)
    {
        // TODO: report ERROR
    }
    else if (NULL == TmraCfg)
    {
        // TODO: report ERROR
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
    uint32_t err = 255;
    if (tmra_isTimerActive(TimerHandlePtr))
    {
        err = (uint32_t) esp_timer_stop(*TimerHandlePtr);
    }
    return err;
}

uint32_t tmra_createTimer(tTMRA_TIMERHANDLE_STR* TimerHandlePtr, void (*timerFunc)(void* arg))
{
    uint32_t err = -1;
    // TODO: Add error detection
    esp_timer_create_args_t* const timerArgs_pstr = (esp_timer_create_args_t*) malloc(sizeof(esp_timer_create_args_t));

    if (NULL != timerArgs_pstr)
    {
        timerArgs_pstr->callback = timerFunc;
        timerArgs_pstr->arg = NULL;
        timerArgs_pstr->dispatch_method = ESP_TIMER_TASK;
        timerArgs_pstr->name = "";
        timerArgs_pstr->skip_unhandled_events = true;
        err = (uint32_t) esp_timer_create(timerArgs_pstr, TimerHandlePtr);
    }
    return err;
}