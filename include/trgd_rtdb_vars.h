#ifndef TRGD_RTDB_VARS_H
#define TRGD_RTDB_VARS_H

#include "rtdb.h"

/*
 * note; min, default and max can be left empty if their value is not limited or relevant
 * [varType]    [arrName]                              [SIZE (optional)]; ///< [unit]  [min]  [default]  [max]    [comment]
 */
tU32S    trgd_ti_us_zeroCrossTriggerDelay_U32;      ///< [us]  [0]  [70]  [1000]   [Time in us between zero-crossing detection and its actual occurrence.]

#endif