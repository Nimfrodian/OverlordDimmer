/*
*
*   timer abstraction file
*
*/
#pragma once

#include "driver/uart.h"
#include "driver/timer.h"

typedef esp_timer_handle_t tTMRA_TIMERHANDLE_STR;

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
 * @brief Function for checking if a timer is active
 * @param TimerHandlePtr pointer to a timer handle
 * @return true if timer is currently active
 */
uint32_t tmra_isTimerActive(tTMRA_TIMERHANDLE_STR* TimerHandlePtr);

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
uint32_t tmra_createTimer(tTMRA_TIMERHANDLE_STR* TimerHandlePtr, void (*timerFunc)(void* arg));