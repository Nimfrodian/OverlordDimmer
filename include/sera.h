/*
*   UART abstraction file
*   used to abstract away UART related names
*
*/

#ifndef SERA_H
#define SERA_H

#include "driver/uart.h"

/**
 * @brief Function initializes Serial Communication
 * @param void
 * @return (void)
 */
void sera_init(void);

#endif