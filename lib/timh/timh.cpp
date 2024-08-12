#include "timh.h"

static bool timh_s_moduleInit_tB = false;
static uint32_t timh_nr_moduleId_U32 = 0;

static tTIMH_INITDATA_STR timh_x_initData_str = {0};
static tTIMH_TIMEDATA_STR timh_ti_timeData_str = {0};

int64_t (*timh_ti_us_sysTimeFunc_pfS64)(void);

void timh_init(tTIMH_INITDATA_STR* TimhCfg)
{
    if (true == timh_s_moduleInit_tB)
    {
        // TODO: report ERROR
    }
    else if (NULL == TimhCfg)
    {
        // TODO: report ERROR
    }
    else
    {
        timh_ti_us_sysTimeFunc_pfS64 = TimhCfg->timh_ti_us_sysTimeFunc_pfS64;

        timh_nr_moduleId_U32 = TimhCfg->nr_moduleId_U32;
        timh_s_moduleInit_tB = true;
    }
}

void timh_canMsgParse_ev(uint8_t* DataPtr, uint32_t* MsgIdPtr)
{
    switch (*MsgIdPtr)
        {
            case (0x94):
            {
                uint16_t year_U16 = ((DataPtr[1] & 0xFF) << 8) | ((DataPtr[0] & 0xFF) << 0);
                uint8_t month_U8 = (DataPtr[2] & 0xFF);
                uint8_t day_U8 = (DataPtr[3] & 0xFF);
                uint8_t hour_U8 = (DataPtr[4] & 0xFF);
                uint8_t minute_U8 = (DataPtr[5] & 0xFF);
                uint8_t second_U8 = (DataPtr[6] & 0xFF);

                timh_ti_timeData_str.year_U16 = year_U16;
                timh_ti_timeData_str.month_U8 = month_U8;
                timh_ti_timeData_str.day_U8 = day_U8;
                timh_ti_timeData_str.hour_U8 = hour_U8;
                timh_ti_timeData_str.minute_U8 = minute_U8;
                timh_ti_timeData_str.second_U8 = second_U8;
            }
            default:
            {
                // TODO: report error
            }
        }
}

tTIMH_TIMEDATA_STR timh_ti_readCurrentTime(void)
{
    return timh_ti_timeData_str;
}

int64_t timh_ti_us_readSystemTime(void)
{
    int64_t ti_us_sysTime = -1;
    if (!timh_s_moduleInit_tB)
    {
        // TODO: report ERROR
    }
    else
    {
        ti_us_sysTime = timh_ti_us_sysTimeFunc_pfS64();
    }
    return ti_us_sysTime;
}