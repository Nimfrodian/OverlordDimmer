#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "driver/timer.h"
#include "driver/uart.h"
#include "string.h"
#include "main.h"
#include "ComCan.h"
#include "logic.h"
#include "esp_log.h"

static uint32_t main_c_numOfTriggeringPins_U32 = 0;
static bool main_f_updateTable_B = true;

gpio_num_t g_triggerPins[] =
{
    GPIO_NUM_16,
    GPIO_NUM_17,
    GPIO_NUM_18,
    GPIO_NUM_19,
    GPIO_NUM_21,
    GPIO_NUM_22,
    GPIO_NUM_23,
    GPIO_NUM_25,
    GPIO_NUM_26,
    GPIO_NUM_27,
};


extern "C" void app_main()
{
    // Initialize UART0
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

    {
        // logic init
        main_c_numOfTriggeringPins_U32 = sizeof(g_triggerPins) / sizeof(g_triggerPins[0]);
        logicInitType logicCfg
        {
            .numOfPhyOut = main_c_numOfTriggeringPins_U32,
        };
        logic_init(&logicCfg);
    }

    {
        gpio_set_level(g_triggerPins[0], 0);
        gpio_set_level(g_triggerPins[1], 0);
        gpio_set_level(g_triggerPins[2], 0);
        gpio_set_level(g_triggerPins[3], 0);
        gpio_set_level(g_triggerPins[4], 0);
        gpio_set_level(g_triggerPins[5], 0);
        gpio_set_level(g_triggerPins[6], 0);
        gpio_set_level(g_triggerPins[7], 0);
        gpio_set_level(g_triggerPins[8], 0);
        gpio_set_level(g_triggerPins[9], 0);

        for (int i = 0; i < main_c_numOfTriggeringPins_U32; i++)
        {
            gpio_reset_pin(g_triggerPins[i]);
            gpio_intr_disable(g_triggerPins[i]);
            gpio_pulldown_dis(g_triggerPins[i]);
            gpio_pullup_dis(g_triggerPins[i]);
            gpio_set_direction(g_triggerPins[i], GPIO_MODE_OUTPUT);
        }

        gpio_pulldown_en(GPIO_NUM_34);
        gpio_set_direction(GPIO_NUM_34, GPIO_MODE_INPUT);
        gpio_set_intr_type(GPIO_NUM_34, GPIO_INTR_POSEDGE);
        gpio_intr_enable(GPIO_NUM_34);

        gpio_install_isr_service(0); // install the ISR service with default configuration
        //gpio_isr_handler_add(GPIO_NUM_34, gpio_isr_handler, (void*) GPIO_NUM_34); // add the custom ISR handler
    }

    ComCan_init();

    while(true)
    {
        if (true == main_f_updateTable_B)
        {
            // calc new timings and masks given the current period ratio
        }

        ComCan_receive();
        canRxDataType* rxDataPtr = ComCan_get_rxData();
        if (1 == rxDataPtr->dataReceived)
        {
            for (int i = 0; i < main_c_numOfTriggeringPins_U32; i++)
            {
                if ((rxDataPtr->triggerIndexMask >> i) & 0x01)
                {
                    float endPrcnt = ((float) rxDataPtr->dutyCycleReq) / 1000.0f;
                    endPrcnt = (endPrcnt > 1.0f)? 1.0f : (endPrcnt < 0.0f) ? 0.0f : endPrcnt;
                    config_duty_cycle(i, endPrcnt, rxDataPtr->timeRequest_10ms);
                }
            }
            rxDataPtr->dataReceived = 0;
        }

        static uint32_t counter = 0;
        if (counter > 100)
        {
            CAN_frame_t canMsg
            {
                .FIR =
                {
                    .B =
                    {
                        .DLC = 8,
                        .unknown_2 = 0,
                        .RTR = CAN_no_RTR,
                        .FF = CAN_frame_std,
                        .reserved_24 = 0,
                    }
                },
                .MsgID = 0x7FF,
                .data = {.u8 = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}
            };

            logic_compose(&canMsg.data.u8[0], &canMsg.MsgID);
            ComCan_transmit(&canMsg, 1);
            counter = 0;
        }
        counter += 5;

        vTaskDelay(5 / portTICK_PERIOD_MS);
    }
}