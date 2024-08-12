#include "pina.h"

static bool pina_s_moduleInit_tB = false;
static uint32_t pina_nr_moduleId_U32 = 0;

void pina_init(tPINA_INITDATA_STR* PinaCfg)
{
    if (true == pina_s_moduleInit_tB)
    {
        // TODO: report ERROR
    }
    else if (NULL == PinaCfg)
    {
        // TODO: report ERROR
    }
    else
    {
        pina_nr_moduleId_U32 = PinaCfg->nr_moduleId_U32;
        pina_s_moduleInit_tB = true;
    }
}

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