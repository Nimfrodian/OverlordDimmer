#include "errh.h"

static bool errh_s_moduleInit_tB = false;
static uint32_t errh_nr_moduleId_U32 = 0;

static std::vector<tERRH_ERRORDATA_STR> errh_x_errors_vstr(ERRH_NR_ERROR_BUFFER_SIZE_U32);  // list of currently active errors
static uint32_t errh_nr_activeErrorCount_U32 = 0;                                           // number of (unique) logged errors

void errh_init(tERRH_INITDATA_STR* ErrhCfg)
{
    if (true == errh_s_moduleInit_tB)
    {
        errh_reportError(ERRH_NOTIF, errh_nr_moduleId_U32, 0, ERRH_API_INIT_U32, ERRH_MODULE_ALREADY_INIT);
    }
    else if (NULL == ErrhCfg)
    {
        errh_x_errors_vstr[0].moduleId = -1;
        errh_x_errors_vstr[0].instanceId = -1;
        errh_x_errors_vstr[0].apiId = -1;
        errh_x_errors_vstr[0].errorId = -1;
        errh_x_errors_vstr[0].errorLvl = ERRH_ERROR_CRITICAL;
        errh_x_errors_vstr[0].count_U8 = (255 == errh_x_errors_vstr[0].count_U8) ? 255 : (errh_x_errors_vstr[0].count_U8 + 1);
        errh_nr_activeErrorCount_U32 = 1;
    }
    else
    {
        errh_nr_moduleId_U32 = ErrhCfg->nr_moduleId_U32;
        errh_s_moduleInit_tB = true;
    }
}

void errh_reportError(tERRH_ERRORTYPE_E ErrorLvl, uint32_t ModuleId, uint32_t InstanceId, uint32_t ApiId, uint32_t ErrorId)
{
    // if trying to write into last place, instead log that memory is full
    if ((ERRH_NR_ERROR_BUFFER_SIZE_U32 - 1) <= errh_nr_activeErrorCount_U32)
    {
        ErrorLvl = ERRH_WARNING;
        ModuleId = -1;
        InstanceId = -1;
        ApiId = -1;
        ErrorId = -1;
    }

    {
        // find if error already exists in the database, and if so, update timestamp and count
        bool moduleAlreadyPresent_tB = false;
        for (uint32_t i_U32 = 0; i_U32 < errh_nr_activeErrorCount_U32; i_U32++)
        {
            if ((ErrorLvl == errh_x_errors_vstr[i_U32].errorLvl) &&
                (ModuleId == errh_x_errors_vstr[i_U32].moduleId) &&
                (InstanceId == errh_x_errors_vstr[i_U32].instanceId) &&
                (ApiId == errh_x_errors_vstr[i_U32].apiId) &&
                (ErrorId == errh_x_errors_vstr[i_U32].errorId)
            )
            {
                if (errh_x_errors_vstr[i_U32].count_U8 < 255)
                {
                    errh_x_errors_vstr[i_U32].count_U8++;
                }
                errh_x_errors_vstr[i_U32].ti_globalTime = timh_ti_readCurrentTime();
                errh_x_errors_vstr[i_U32].ti_us_timestamp = timh_ti_us_readSystemTime_S64();
                moduleAlreadyPresent_tB = true;
            }
        }
        // else save the new error data
        if (false == moduleAlreadyPresent_tB)
        {
            errh_x_errors_vstr[errh_nr_activeErrorCount_U32].moduleId = ModuleId;
            errh_x_errors_vstr[errh_nr_activeErrorCount_U32].instanceId = InstanceId;
            errh_x_errors_vstr[errh_nr_activeErrorCount_U32].apiId = ApiId;
            errh_x_errors_vstr[errh_nr_activeErrorCount_U32].errorId = ErrorId;
            errh_x_errors_vstr[errh_nr_activeErrorCount_U32].errorLvl = ErrorLvl;
            errh_x_errors_vstr[errh_nr_activeErrorCount_U32].ti_globalTime = timh_ti_readCurrentTime();
            errh_x_errors_vstr[errh_nr_activeErrorCount_U32].ti_us_timestamp = timh_ti_us_readSystemTime_S64();
            errh_x_errors_vstr[errh_nr_activeErrorCount_U32].count_U8 = 1;
            errh_nr_activeErrorCount_U32++;
        }
    }
}

tERRH_ERRORDATA_STR errh_readError(uint32_t ErrorIndx)
{
    tERRH_ERRORDATA_STR errData_str =
    {
        .moduleId = 0,
        .instanceId = 0,
        .apiId = 0,
        .errorId = 0,
        .errorLvl = ERRH_ERRORTYPE_UNDEF,
        .ti_globalTime =
        {
            .year_U16 = 0,
            .month_U8 = 0,
            .day_U8 = 0,
            .hour_U8 = 0,
            .minute_U8 = 0,
            .second_U8 = 0,
        },
        .ti_us_timestamp = 0,
        .count_U8 = 0,
    };
    if (errh_nr_activeErrorCount_U32 <= ErrorIndx)
    {
        errh_reportError(ERRH_WARNING, errh_nr_moduleId_U32, 0, ERRH_API_READ_ERROR_U32, ERRH_ERR_READ_INDEX_OUT_OF_BOUNDS_U32);
    }
    else
    {
        errData_str = errh_x_errors_vstr[ErrorIndx];
    }
    return errData_str;
}

uint32_t errh_readReportedErrorCount(void)
{
    return errh_nr_activeErrorCount_U32;
}

void errh_clearErrorCount(void)
{
    errh_nr_activeErrorCount_U32 = 0;
}

void errh_deinit(void)
{
    errh_s_moduleInit_tB = false;

    tERRH_ERRORDATA_STR emptyData_str =
    {
        .moduleId = 0,
        .instanceId = 0,
        .apiId = 0,
        .errorId = 0,
        .errorLvl = ERRH_ERRORTYPE_UNDEF,
        .ti_globalTime =
        {
            .year_U16 = 0,
            .month_U8 = 0,
            .day_U8 = 0,
            .hour_U8 = 0,
            .minute_U8 = 0,
            .second_U8 = 0,
        },
        .ti_us_timestamp = 0,
        .count_U8 = 0,
    };
    for (uint32_t i_U32 = 0; i_U32 < ERRH_NR_ERROR_BUFFER_SIZE_U32; i_U32++)
    {
        errh_x_errors_vstr[i_U32] = emptyData_str;
    }

    errh_clearErrorCount();
}