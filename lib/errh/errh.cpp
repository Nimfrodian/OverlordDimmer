#include "errh.h"

static std::vector<tERRH_ERRORDATA_STR> errh_x_errors_vstr(ERRH_NR_ERROR_BUFFER_SIZE_U32);  // list of currently active errors

static uint32_t errh_nr_activeErrorCount_U32 = 0;

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
        for (std::vector<tERRH_ERRORDATA_STR>::iterator it = errh_x_errors_vstr.begin();
            it != errh_x_errors_vstr.end();
            it++)
        {
            if ((ErrorLvl == it->errorLvl) &&
                (ModuleId == it->moduleId) &&
                (InstanceId == it->instanceId) &&
                (ApiId == it->apiId) &&
                (ErrorId == it->errorId)
            )
            {
                if (it->count_U8 < 255)
                {
                    it->count_U8++;
                }
                it->ti_globalTime = timh_ti_readCurrentTime();
                it->ti_us_timestamp = timh_ti_us_readSystemTime();
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
            errh_x_errors_vstr[errh_nr_activeErrorCount_U32].ti_us_timestamp = timh_ti_us_readSystemTime();
            errh_x_errors_vstr[errh_nr_activeErrorCount_U32].count_U8 = 1;
            errh_nr_activeErrorCount_U32++;
        }
    }
}

tERRH_ERRORDATA_STR errh_readError(uint32_t ErrorIndx)
{
    tERRH_ERRORDATA_STR errData_str = {0};
    if (errh_nr_activeErrorCount_U32 <= ErrorIndx)
    {
        // TODO: report ERROR
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