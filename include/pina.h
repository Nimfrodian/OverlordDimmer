/*
*   pins abstraction file (GPIO)
*
*
*/

#pragma once
#include "driver/gpio.h"


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

void pina_setGpioLevel(PINA_nr_GPIO_NUM_E GpioNum, bool Value)
{
    gpio_set_level((gpio_num_t) GpioNum, Value);
}

void pina_setGpioAsOutput(PINA_nr_GPIO_NUM_E GpioNum)
{
    gpio_reset_pin((gpio_num_t) GpioNum);
    gpio_intr_disable((gpio_num_t) GpioNum);
    gpio_pulldown_dis((gpio_num_t) GpioNum);
    gpio_pullup_dis((gpio_num_t) GpioNum);
    gpio_set_direction((gpio_num_t) GpioNum, GPIO_MODE_OUTPUT);
}

void pina_setGpioAsInput(PINA_nr_GPIO_NUM_E GpioNum)
{
    gpio_pulldown_en((gpio_num_t) GpioNum);
    gpio_set_direction((gpio_num_t) GpioNum, GPIO_MODE_INPUT);
    gpio_set_intr_type((gpio_num_t) GpioNum, GPIO_INTR_POSEDGE);
    gpio_intr_enable((gpio_num_t) GpioNum);
}

void pina_setInterruptService(PINA_nr_GPIO_NUM_E GpioNum, void (*func)(void*arg))
{
    gpio_install_isr_service(0); ///< install the ISR service with default configuration
    gpio_isr_handler_add((gpio_num_t) GpioNum, func, (void*) GpioNum); ///< add the custom ISR handler

}