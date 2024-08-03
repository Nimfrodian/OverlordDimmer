#ifndef LOGIC_H
#define LOGIC_H
#include "stdint.h"
#include "math.h"

/**
 * @brief Function composes duty cycles into a CAN message pointer.
 * @param CanMsgPtr Pointer to the CAN message to be filled
 * @return (void)
 */
void logic_compose(uint8_t* DataPtr, uint32_t* MsgIdPtr);


#endif