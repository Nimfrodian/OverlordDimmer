/*
*
*   Trigger driver for AC mosfet triggering
*
*/
#pragma once

#include "espa.h"
#include "tmra.h"
#include "lgic.h"

typedef struct
{
    uint32_t nr_moduleId_U32;       ///< ID of the module
} tTRGD_INITDATA_STR;

void trgd_init(tTRGD_INITDATA_STR* TrgdCfg);
void trgd_run(void);