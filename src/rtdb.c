#include"rtdb.h"


void rtdb_assign_tU32S( tU32S* VarAddrs,
                         tU32 Indx,
                         unitEnumT Unit,
                         tU32 Min,
                         tU32 Def,
                         tU32 Max,
                         char* Comment )
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