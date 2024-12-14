/*
*
*   Trigger driver for AC mosfet triggering
*
*/
#pragma once

#include "espa.h"
#include "tmra.h"
#include "lgic.h"
#include "errh.h"
#include "mdll.h"

#define TRGD_API_INIT_U32                   ((uint32_t) 1)
#define TRGD_API_INITIAL_INTRPT_HANDLER_U32 ((uint32_t) 2)
#define TRGD_API_APPLY_OUTPUT_U32           ((uint32_t) 3)

#define TRGD_ERR_CANNOT_STOP_TIMER_U32      ((uint32_t) 1)
#define TRGD_ERR_CANNOT_START_TIMER         ((uint32_t) 2)


typedef struct
{
    uint32_t nr_moduleId_U32;       ///< ID of the module
} tTRGD_INITDATA_STR;

void trgd_init(tTRGD_INITDATA_STR* TrgdCfg);
void trgd_run_5ms(void);