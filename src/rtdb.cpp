#include"rtdb.h"
#include "trgd_rtdb.h"

static bool rtdb_s_moduleInit_tB = false;
static uint32_t rtdb_nr_moduleId_U32 = 0;

const char* rtdb_unitLookupTable[VAR_UNIT_MAX_NUM] =
{
    [VAR_UNIT_us] = "us",
};

tU32S* rtdb_arr_tU32S[NUM_OF_TU32S] = {0};

tU32 rtdb_calcCrc32 (void* VarAddrs, tU32 VarSize )
{
    // TODO
    return -1;
}

void rtdb_assign_tU32S( tU32S* VarAddrs,
                         tU32 Indx,
                         unitEnumT Unit,
                         tU32 Min,
                         tU32 Def,
                         tU32 Max,
                         const char* Comment )
{
    rtdb_arr_tU32S[Indx] = VarAddrs; // Copy address of variable to lookup table
    VarAddrs->tU32_val = Def;
    VarAddrs->tU32_min = Min;
    VarAddrs->tU32_def = Def;
    VarAddrs->tU32_max = Max;
    VarAddrs->ss.signalUnit = Unit;
    VarAddrs->ss.signalCmnt = Comment;
    VarAddrs->ss.signalState = SIGNAL_OK;
    VarAddrs->ss.objectStatus = OBJECT_STANDARD;
    VarAddrs->ss.crc32 = rtdb_calcCrc32(VarAddrs, sizeof(tU32S));
};

tU32 rtdb_write_tU32S( tU32S* VarAddrs, tU32 NewVal )
{
    tU32 stsCode = 0;
    tU32 valToWrite = VarAddrs->tU32_def;
    // check CRC
    if (rtdb_checkCrc32(VarAddrs, sizeof(VarAddrs)))
    {
        // TODO
    }
    // check for MIN
    else if ( NewVal < VarAddrs->tU32_min)
    {
        // TODO: Report error
        stsCode = 2;
    }
    // check for MAX
    else if ( NewVal > VarAddrs->tU32_max)
    {
        // TODO: Report error
        stsCode = 3;
    }
    else
    {
        if (OBJECT_STANDARD == VarAddrs->ss.objectStatus)
        {
            valToWrite = NewVal;
            rtdb_calcCrc32( VarAddrs, sizeof(tU32S));
        }
    }

    VarAddrs->tU32_val = valToWrite;
    return stsCode;
}

tU32 rtdb_overwrite_tU32S( tU32S* VarAddrs, tU32 NewVal )
{
    tU32 stsCode = 0;
    tU32 valToWrite = VarAddrs->tU32_def;
    // check CRC
    if (rtdb_checkCrc32( VarAddrs, sizeof(VarAddrs))) // TODO
    {
        // TODO
        stsCode = 1;
    }
    else
    {
        valToWrite = NewVal;
        VarAddrs->ss.objectStatus = OBJECT_OVERRIDDEN;
    }

    VarAddrs->tU32_val = valToWrite;

    //Recalculate CRC32
    rtdb_calcCrc32( VarAddrs, sizeof(tU32S));
    return stsCode;
}

tU32 rtdb_releaseOverwrite_tU32S( tU32S* VarAddrs )
{
    tU32 stsCode = 0;
    // check CRC
    if (0) // TODO
    {
        // TODO
        stsCode = 1;
    }
    else
    {
        VarAddrs->ss.objectStatus = OBJECT_STANDARD;
        // TODO: Recalculate CRC
    }

    return stsCode;
}

/**
* @brief Function returns current value of given variable
* @param VarAddrs address to the underlying variable
* @return Verified value of the given variable
*/
tU32 rtdb_read_tU32S( tU32S* VarAddrs )
{
    tU32 toReturn = VarAddrs->tU32_val;
    // Check CRC
    if (0)
    {
        // TODO: implement CRC failed case
    }

    return toReturn;
}

void rtdb_init(tRTDB_INITDATA_STR* RtdbCfg)
{
    if (true == rtdb_s_moduleInit_tB)
    {
        errh_reportError(ERRH_NOTIF, rtdb_nr_moduleId_U32, 0, RTDB_API_INIT_U32, ERRH_MODULE_ALREADY_INIT);
    }
    else if (NULL == RtdbCfg)
    {
        errh_reportError(ERRH_ERROR_CRITICAL, rtdb_nr_moduleId_U32, 0, RTDB_API_INIT_U32, ERRH_POINTER_IS_NULL);
    }
    else
    {
        rtdb_s_moduleInit_tB = true;
        trgd_rtdb_init();
    }
}
