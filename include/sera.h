/*
*   UART abstraction file
*   used to abstract away UART related names
*
*/
#pragma once

#include "driver/uart.h"
#include "errh.h"

#define SERA_API_INIT_U32                  ((uint32_t) 1)

typedef struct
{
    uint32_t nr_moduleId_U32;       ///< ID of the module
} tSERA_INITDATA_STR;

/**
 * @brief Function initializes Serial Communication
 * @param void
 * @return (void)
 */
void sera_init(void);