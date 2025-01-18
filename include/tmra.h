/*
*
*   timer abstraction file
*
*/
#pragma once

#include "driver/uart.h"
#include "driver/gptimer.h"
#include "errh.h"

#define TMRA_API_INIT_U32                  ((uint32_t) 1)
#define TMRA_API_CREATE_TIMER_U32          ((uint32_t) 2)
#define TMRA_API_START_TIMER_U32           ((uint32_t) 3)
#define TMRA_API_STOP_TIMER_U32            ((uint32_t) 4)

#define TMRA_ERR_MALLOC_RETURNED_NULL_U32  ((uint32_t) 1)
#define TMRA_ERR_CANNOT_CREATE_TIMER_U32   ((uint32_t) 2)
#define TMRA_ERR_CANNOT_ENABLE_TIMER_U32   ((uint32_t) 3)
#define TMRA_ERR_CANNOT_SET_ALARM_U32      ((uint32_t) 4)
#define TMRA_ERR_CANNOT_START_TIMER_U32    ((uint32_t) 5)
#define TMRA_ERR_CANNOT_DISABLE_TIMER_U32  ((uint32_t) 6)
#define TMRA_ERR_CANNOT_STOP_TIMER_U32     ((uint32_t) 7)


typedef struct
{
    uint32_t nr_moduleId_U32;       ///< ID of the module
} tTMRA_INITDATA_STR;

typedef gptimer_handle_t tTMRA_TIMERHANDLE_STR;

/**
 * @brief Functions returns the time in microseconds since program start
 * @param void
 * @return time in microseconds since program start
 */
int64_t tmra_ti_us_getCurrentTime_S64(void);

/**
 * @brief Function starts a timer with a given delay. Once the time is up the function is executed
 * @param TimerHandlePtr Pointer to a timer handle
 * @param Ti_us_delay Time to non-blockingly delay the function execution
 * @return error code
 */
uint32_t tmra_startTimer(tTMRA_TIMERHANDLE_STR* TimerHandlePtr, uint32_t Ti_us_delay);

/**
 * @brief Function stops a timer
 * @param TimerHandlePtr pointer to a timer handle
 * @return error code
 */
uint32_t tmra_stopTimer(tTMRA_TIMERHANDLE_STR* TimerHandlePtr);

/**
 * @brief Function setups a timer
 * @param TimerHandlePtr pointer to a timer handle
 * @param timerFunc function to be executed
 * @return error code
 */
uint32_t tmra_createTimer(tTMRA_TIMERHANDLE_STR* TimerHandlePtr, bool (*timerFunc)(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data));