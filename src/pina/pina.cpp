#include "pina.h"

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