#include "sera.h"

static bool sera_s_moduleInit_tB = false;
static uint32_t sera_nr_moduleId_U32 = 0;

void sera_init(tSERA_INITDATA_STR* SeraCfg)
{
    if (true == sera_s_moduleInit_tB)
    {
        // TODO: report ERROR
    }
    else if (NULL == SeraCfg)
    {
        // TODO: report ERROR
    }
    else
    {
        ///< Initialize UART0
        uart_config_t uart_config = {
            .baud_rate = 115200,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
            .rx_flow_ctrl_thresh = 0,
            .source_clk = UART_SCLK_APB,
        };
        uart_param_config(UART_NUM_0, &uart_config);
        uart_driver_install(UART_NUM_0, 1024 * 2, 0, 0, NULL, 0);

        sera_nr_moduleId_U32 = SeraCfg->nr_moduleId_U32;
        sera_s_moduleInit_tB = true;
    }
}