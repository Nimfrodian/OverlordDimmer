/*
*
*   Time handling module
*
*/
#pragma once

#include "stdint.h"

typedef struct
{
    int64_t (*sysTimeFunc)(void);   // pointer to system time function
} tTIMH_INITDATA_STR;

typedef struct
{
    uint16_t year_U16;              ///< year
    uint8_t month_U8;              ///< month
    uint8_t day_U8;                ///< day
    uint8_t hour_U8;               ///< hour
    uint8_t minute_U8;             ///< minute
    uint8_t second_U8;             ///< second
} tTIMH_TIMEDATA_STR;

/**
 * @brief Function initialized timh module
 * @param InitData_pstr
 * @return (void)
 */
void timh_init(tTIMH_INITDATA_STR* InitData_pstr);

/**
 * @brief Function parses CAN messages related to timh module. Message should arrive at least every second
 * @param DataPtr pointer to data to be parsed
 * @param MsgIdPtr pointer to message ID
 * @return (void)
 */
void timh_canMsgParse_ev(uint8_t* DataPtr, uint32_t* MsgIdPtr);

/**
 * @brief Function returns current (real) time
 * @param void
 * @return copy of tTIMH_TIMEDATA_STR with current real world time
 */
tTIMH_TIMEDATA_STR timh_ti_readCurrentTime(void);

/**
 * @brief Function returns system time since reset in us
 * @param void
 * @return system time in us since reset
 */
int64_t timh_ti_us_readSystemTime(void);