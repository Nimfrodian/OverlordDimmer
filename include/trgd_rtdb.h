#ifndef TRGD_RTDB_H
#define TRGD_RTDB_H

#include "rtdb.h"
#include "trgd_rtdb_vars.h"

void trgd_rtdb_init()
{
    rtdb_assign_tU32S(&trgd_ti_us_zeroCrossTriggerDelay_U32, TRGD_TI_US_ZEROCROSSTRIGGERDELAY_U32, VAR_UNIT_us, 0, 70, 1000, "Time in us between zero-crossing detection and its actual occurrence.");
}
#endif
