#include "dmas.h"
#include "errh.h"
#include "rtdb.h"

static bool dmas_s_moduleInit_tB = false;
static uint32_t dmas_nr_moduleId_U32 = 0;


static tDMAS_STREAMING_STR dmas_x_streamingData_astr[DMAS_MAX_NUM_OF_STREAMABLE_VARS] = {0};

void dmas_init(tDMAS_INITDATA_STR* dmasCfg)
{
    if (true == dmas_s_moduleInit_tB)
    {
        errh_reportError(ERRH_NOTIF, dmas_nr_moduleId_U32, 0, DMAS_API_INIT_U32, ERRH_MODULE_ALREADY_INIT);
    }
    else if (NULL == dmasCfg)
    {
        errh_reportError(ERRH_ERROR_CRITICAL, dmas_nr_moduleId_U32, 0, DMAS_API_INIT_U32, ERRH_POINTER_IS_NULL);
    }
    else
    {
        dmas_nr_moduleId_U32 = dmasCfg->nr_moduleId_U32;
        dmas_s_moduleInit_tB = true;    // only init once
    }
}

/**
 * @brief Runnable for DMAS module which allows for internal variable streaming and setting
 * @param void
 * @return (void)
 */
void dmas_run_5ms(tDMAS_INPUTDATA_STR* InputPtr, tDMAS_OUTPUTDATA_STR* OutputPtr)
{
    switch( InputPtr->payload_tU8[DMAS_MODE_BYTE] )
    {
        case DMAS_MODE_READ:
        {

            break;
        }
        case DMAS_MODE_STOP_READ:
        {

            break;
        }
        case DMAS_MODE_WRITE:
        {
            tU32 errCode = 0;
            tU8* ptr = InputPtr->payload_tU8;
            tU32 rawNewVal_tU32 = (ptr[7] << 24) | (ptr[6] << 16) | (ptr[5] << 8) | (ptr[4] << 0);
            switch (InputPtr->payload_tU8[DMAS_WRITE_VAR_TYPE])
            {
                case DMAS_TU8:
                {
                    errCode = rtdb_write_tU8S((tU8SEnumT) InputPtr->payload_tU8[DMAS_VAR_INDEX], (tU8) rawNewVal_tU32);
                    break;
                }
                case DMAS_TU16:
                {
                    errCode = rtdb_write_tU16S((tU16SEnumT) InputPtr->payload_tU8[DMAS_VAR_INDEX], (tU16) rawNewVal_tU32);
                    break;
                }
                case DMAS_TU32:
                {
                    errCode = rtdb_write_tU32S((tU32SEnumT) InputPtr->payload_tU8[DMAS_VAR_INDEX], (tU32) rawNewVal_tU32);
                    break;
                }
                case DMAS_TS8:
                {
                    errCode = rtdb_write_tS8S((tS8SEnumT) InputPtr->payload_tU8[DMAS_VAR_INDEX], (tS8) rawNewVal_tU32);
                    break;
                }
                case DMAS_TS16:
                {
                    errCode = rtdb_write_tS16S((tS16SEnumT) InputPtr->payload_tU8[DMAS_VAR_INDEX], (tS16) rawNewVal_tU32);
                    break;
                }
                case DMAS_TS32:
                {
                    errCode = rtdb_write_tS32S((tS32SEnumT) InputPtr->payload_tU8[DMAS_VAR_INDEX], (tS32) rawNewVal_tU32);
                    break;
                }
                case DMAS_TF32S:
                {
                    errCode = rtdb_write_tF32S((tF32SEnumT) InputPtr->payload_tU8[DMAS_VAR_INDEX], (tF32) rawNewVal_tU32);
                    break;
                }
                case DMAS_TBS:
                {
                    errCode = rtdb_write_tBS((tBSEnumT) InputPtr->payload_tU8[DMAS_VAR_INDEX], (tB) rawNewVal_tU32);
                    break;
                }
                case DMAS_TES:
                {
                    errCode = rtdb_write_tES((tESEnumT) InputPtr->payload_tU8[DMAS_VAR_INDEX], (tE) rawNewVal_tU32);
                    break;
                }
                default:
                {
                    errh_reportError(ERRH_NOTIF, dmas_nr_moduleId_U32, 0, DMAS_API_RUN_U32, DMAS_ERR_WRONG_UNIT_TYPE);
                    break;
                }

                if (errCode)
                {
                    errh_reportError(ERRH_NOTIF, dmas_nr_moduleId_U32, 0, DMAS_API_RUN_U32, DMAS_ERR_RTDB_WRITE_ERR);
                }
            }
            break;
        }
        case DMAS_MODE_OVERWRITE:
        {
            tU32 errCode = 0;
            tU8* ptr = InputPtr->payload_tU8;
            tU32 rawNewVal_tU32 = (ptr[7] << 24) | (ptr[6] << 16) | (ptr[5] << 8) | (ptr[4] << 0);
            switch (InputPtr->payload_tU8[DMAS_WRITE_VAR_TYPE])
            {
                case DMAS_TU8:
                {
                    errCode = rtdb_overwrite_tU8S((tU8SEnumT) InputPtr->payload_tU8[DMAS_VAR_INDEX], (tU8) rawNewVal_tU32);
                    break;
                }
                case DMAS_TU16:
                {
                    errCode = rtdb_overwrite_tU16S((tU16SEnumT) InputPtr->payload_tU8[DMAS_VAR_INDEX], (tU16) rawNewVal_tU32);
                    break;
                }
                case DMAS_TU32:
                {
                    errCode = rtdb_overwrite_tU32S((tU32SEnumT) InputPtr->payload_tU8[DMAS_VAR_INDEX], (tU32) rawNewVal_tU32);
                    break;
                }
                case DMAS_TS8:
                {
                    errCode = rtdb_overwrite_tS8S((tS8SEnumT) InputPtr->payload_tU8[DMAS_VAR_INDEX], (tS8) rawNewVal_tU32);
                    break;
                }
                case DMAS_TS16:
                {
                    errCode = rtdb_overwrite_tS16S((tS16SEnumT) InputPtr->payload_tU8[DMAS_VAR_INDEX], (tS16) rawNewVal_tU32);
                    break;
                }
                case DMAS_TS32:
                {
                    errCode = rtdb_overwrite_tS32S((tS32SEnumT) InputPtr->payload_tU8[DMAS_VAR_INDEX], (tS32) rawNewVal_tU32);
                    break;
                }
                case DMAS_TF32S:
                {
                    errCode = rtdb_overwrite_tF32S((tF32SEnumT) InputPtr->payload_tU8[DMAS_VAR_INDEX], (tF32) rawNewVal_tU32);
                    break;
                }
                case DMAS_TBS:
                {
                    errCode = rtdb_overwrite_tBS((tBSEnumT) InputPtr->payload_tU8[DMAS_VAR_INDEX], (tB) rawNewVal_tU32);
                    break;
                }
                case DMAS_TES:
                {
                    errCode = rtdb_overwrite_tES((tESEnumT) InputPtr->payload_tU8[DMAS_VAR_INDEX], (tE) rawNewVal_tU32);
                    break;
                }
                default:
                {
                    errh_reportError(ERRH_NOTIF, dmas_nr_moduleId_U32, 0, DMAS_API_RUN_U32, DMAS_ERR_WRONG_UNIT_TYPE);
                    break;
                }

                if (errCode)
                {
                    errh_reportError(ERRH_NOTIF, dmas_nr_moduleId_U32, 0, DMAS_API_RUN_U32, DMAS_ERR_RTDB_OVERWRITE_ERR);
                }
            }
            break;
        }
        case DMAS_MODE_STOPOVERWRITE:
        {
            tU32 errCode = 0;
            switch (InputPtr->payload_tU8[DMAS_WRITE_VAR_TYPE])
            {
                case DMAS_TU8:
                {
                    errCode = rtdb_releaseOverwrite_tU8S((tU8SEnumT) InputPtr->payload_tU8[DMAS_VAR_INDEX]);
                    break;
                }
                case DMAS_TU16:
                {
                    errCode = rtdb_releaseOverwrite_tU16S((tU16SEnumT) InputPtr->payload_tU8[DMAS_VAR_INDEX]);
                    break;
                }
                case DMAS_TU32:
                {
                    errCode = rtdb_releaseOverwrite_tU32S((tU32SEnumT) InputPtr->payload_tU8[DMAS_VAR_INDEX]);
                    break;
                }
                case DMAS_TS8:
                {
                    errCode = rtdb_releaseOverwrite_tS8S((tS8SEnumT) InputPtr->payload_tU8[DMAS_VAR_INDEX]);
                    break;
                }
                case DMAS_TS16:
                {
                    errCode = rtdb_releaseOverwrite_tS16S((tS16SEnumT) InputPtr->payload_tU8[DMAS_VAR_INDEX]);
                    break;
                }
                case DMAS_TS32:
                {
                    errCode = rtdb_releaseOverwrite_tS32S((tS32SEnumT) InputPtr->payload_tU8[DMAS_VAR_INDEX]);
                    break;
                }
                case DMAS_TF32S:
                {
                    errCode = rtdb_releaseOverwrite_tF32S((tF32SEnumT) InputPtr->payload_tU8[DMAS_VAR_INDEX]);
                    break;
                }
                case DMAS_TBS:
                {
                    errCode = rtdb_releaseOverwrite_tBS((tBSEnumT) InputPtr->payload_tU8[DMAS_VAR_INDEX]);
                    break;
                }
                case DMAS_TES:
                {
                    errCode = rtdb_releaseOverwrite_tES((tESEnumT) InputPtr->payload_tU8[DMAS_VAR_INDEX]);
                    break;
                }
                default:
                {
                    errh_reportError(ERRH_NOTIF, dmas_nr_moduleId_U32, 0, DMAS_API_RUN_U32, DMAS_ERR_WRONG_UNIT_TYPE);
                    break;
                }

                if (errCode)
                {
                    errh_reportError(ERRH_NOTIF, dmas_nr_moduleId_U32, 0, DMAS_API_RUN_U32, DMAS_ERR_RTDB_RELEASEOVERWRITE_ERR);
                }
            }
            break;
        }
        default:
        {
            errh_reportError(ERRH_NOTIF, dmas_nr_moduleId_U32, 0, DMAS_API_RUN_U32, DMAS_ERR_WRONG_COMMAND_TYPE);
            break;
        }
    }
}