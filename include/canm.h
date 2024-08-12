/*
*
*   Can module
*
*/
#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <driver/twai.h>    // CAN protocol library, a.k.a. twin wire automotive interface
#include "errh.h"

#define CANM_API_INIT_U32                  ((uint32_t) 1)

#define CANM_TI_ms_TASK_DELAY_U32 ((uint32_t) 2)

typedef struct
{
    uint32_t nr_moduleId_U32;       ///< ID of the module
} tCANM_INITDATA_STR;

typedef struct
{
    twai_message_t canMsg_str;     // CAN message structure
    bool canRdyForTx_tB;       // message ready for transmission flag
    bool canRdyForParse_tB;    // message ready to be parsed. Only applicable for RX messages
} tCANM_X_CANMSGDATA_STR;

typedef enum
{
    CAN_REPLY_MESSAGE0 = 0,
    CAN_REPLY_MESSAGE1,
    CAN_COMMAND_MESSAGE,
    NUM_OF_CAN_MSG
} tCANM_CANMSGINDX_E;


/**
 * @brief Initialize CAN module
 * @param void
 * @return (void)
 */
void canm_init(tCANM_INITDATA_STR* CanmCfg);

/**
 * @brief Function returns pointer to CAN message data
 * @param msgIndx CAN message index to access
 * @return pointer to CAN message data
 */
tCANM_X_CANMSGDATA_STR* canm_pstr_readCanMsgData(tCANM_CANMSGINDX_E msgIndx);

/**
 * @brief Flag message for transmission
 * @param MsgIndx CAN message to transmit
 * @return (void)
 */
void canm_x_flagCanMsgForTx(tCANM_CANMSGINDX_E MsgIndx);

/**
 * @brief Clear parsing flag for CAN message. Should be done after parsing it
 * @param MsgIndx CAN message to clear parse flag for
 * @return (void)
 */
void canm_x_clearFlagForCanMsgParse(tCANM_CANMSGINDX_E MsgIndx);

/**
 * @brief Transceive function for CAN message tx/rx
 * @param param
 * @return (void)
 */
void canm_transceive_run(void);