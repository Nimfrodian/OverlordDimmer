/*
*   pins abstraction file (GPIO)
*
*
*/
#pragma once

#include "driver/gpio.h"

typedef struct
{
    uint32_t nr_moduleId_U32;       ///< ID of the module
} tPINA_INITDATA_STR;

typedef enum
{
    // OUTPUTS
    PINA_OUT_NUM_0 = GPIO_NUM_16,
    PINA_OUT_NUM_1 = GPIO_NUM_17,
    PINA_OUT_NUM_2 = GPIO_NUM_18,
    PINA_OUT_NUM_3 = GPIO_NUM_19,
    PINA_OUT_NUM_4 = GPIO_NUM_21,
    PINA_OUT_NUM_5 = GPIO_NUM_22,
    PINA_OUT_NUM_6 = GPIO_NUM_23,
    PINA_OUT_NUM_7 = GPIO_NUM_25,
    PINA_OUT_NUM_8 = GPIO_NUM_26,
    PINA_OUT_NUM_9 = GPIO_NUM_27,

    // INPUTS
    PINA_IN_NUM_0 = GPIO_NUM_34,
} PINA_nr_GPIO_NUM_E;

/**
 * @brief Function initializes pin abstraction handling module
 * @param
 * @return (void)
 */
void pina_init(tPINA_INITDATA_STR* PinaCfg);

/**
 * @brief Function sets GPIO output to desired value
 * @param GpioNum GPIO pin to set value to
 * @param Value value to apply
 * @return (void)
 */
void pina_setGpioLevel(PINA_nr_GPIO_NUM_E GpioNum, bool Value);

/**
 * @brief Function sets GPIO pin as output
 * @param GpioNum GPIO number
 * @return (void)
 */
void pina_setGpioAsOutput(PINA_nr_GPIO_NUM_E GpioNum);

/**
 * @brief Function sets GPIO pin as input
 * @param GpioNum GPIO number
 * @return (void)
 */
void pina_setGpioAsInput(PINA_nr_GPIO_NUM_E GpioNum);

/**
 * @brief Function sets GPIO pin for interrupt service
 * @param GpioNum GPIO number
 * @param func pointer to a function to be executed on GPIO interrupt
 * @return (void)
 */
void pina_setInterruptService(PINA_nr_GPIO_NUM_E GpioNum, void (*func)(void*arg));