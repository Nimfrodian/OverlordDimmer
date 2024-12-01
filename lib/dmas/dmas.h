
/*
 * Direct Memory Access Server module
 * Module is used in conjunction with Real Time Database (RTDB) module to access RAM memory during runtime
 * Supports streaming of up to 'DMAS_MAX_NUM_OF_STREAMABLE_VARS'
 */

#pragma once
#include "rtdb_vars.h"


#define DMAS_API_INIT_U32                  ((tU32) 1)
#define DMAS_API_RUN_U32                   ((tU32) 2)

#define DMAS_ERR_WRONG_COMMAND_TYPE        ((tU32) 1)
#define DMAS_ERR_WRONG_UNIT_TYPE           ((tU32) 2)
#define DMAS_ERR_RTDB_WRITE_ERR            ((tU32) 3)
#define DMAS_ERR_RTDB_OVERWRITE_ERR        ((tU32) 4)
#define DMAS_ERR_RTDB_RELEASEOVERWRITE_ERR ((tU32) 5)

#define DMAS_MAX_NUM_OF_STREAMABLE_VARS ((tU8) 16)

typedef enum
{
    DMAS_MODE_READ = 0,         ///< Request to stream variable
    DMAS_MODE_STOP_READ,        ///< Request to stop streaming variable
    DMAS_MODE_WRITE,            ///< Request to one-time write to variable
    DMAS_MODE_OVERWRITE,        ///< Request to overwrite to variable, preventing program from updating variable
    DMAS_MODE_STOPOVERWRITE,    ///< Request to release overwrite lock to allow program to take over
} tDMAS_MODETYPE_E;

typedef enum
{
    DMAS_TU8  = 0,
    DMAS_TU16,
    DMAS_TU32,

    DMAS_TS8 ,
    DMAS_TS16,
    DMAS_TS32,

    DMAS_TF32S,
    DMAS_TBS,
    DMAS_TES,
} tDMAS_VARTYPE_E;

enum
{
    DMAS_MODE_BYTE = 0,         // Mode selection byte as per tDMAS_MODETYPE_E

    DMAS_WRITE_VAR_TYPE = 1,    // Variable type to be written byte as per tDMAS_VARTYPE_E
    DMAS_VAR_INDEX = 2,         // Variable index to write into

};

typedef struct
{
    uint32_t nr_moduleId_U32;   ///< ID of the module
} tDMAS_INITDATA_STR;

typedef struct
{
    tU8 payload_tU8[8];     ///< CAN message payload to be parsed
} tDMAS_INPUTDATA_STR;

typedef struct
{
    tU32 varIndex_tU32;             ///< variable index to stream, as defined in rtdb_vars.h
    tDMAS_VARTYPE_E varType_tU32;   ///< variable type (tU8, tU16...)
    tU32 ti_ms_varInterval_tU32;    ///< Streaming interval in ms
} tDMAS_STREAMING_STR;

typedef struct
{
    tU8 payload_tU8[DMAS_MAX_NUM_OF_STREAMABLE_VARS][8];    ///< CAN message payloads to be sent
} tDMAS_OUTPUTDATA_STR;



void dmas_init(tDMAS_INITDATA_STR* dmasCfg);
void dmas_run_5ms(tDMAS_INPUTDATA_STR*, tDMAS_OUTPUTDATA_STR*);