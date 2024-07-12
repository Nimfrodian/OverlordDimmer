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

#define NUM_OF_TRIGGERS 10

esp_timer_handle_t singleShot_timerOn;
esp_timer_handle_t singleShot_timerDelay;
triggerTableType g_triggerTable[2][NUM_OF_TRIGGERS + 1] = {0}; // two trigger tables - one active, one in preparation. 1 initial state and 1 state for each output
triggerTableType* g_activeTable = &g_triggerTable[1][0];    // active table currently being applied
triggerTableType* g_preparingTable = &g_triggerTable[0][0]; // table being prepared and will be used in the next cycle
bool g_updateTable_flg = false;


gpio_num_t g_triggerPins[NUM_OF_TRIGGERS] =
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

volatile uint32_t g_triggerCounter = 0;


void IRAM_ATTR gpio_isr_handler(void* arg)
{
    // start countdown timer as the zero-cross trigger occurs some time before actual zero-cross
    ESP_ERROR_CHECK(esp_timer_start_once(singleShot_timerDelay, 440));
}

void timer_DELAY_isr_handler(void* arg)
{
    static uint64_t lastTriggerTime = 0;
    uint64_t currTriggerTime = esp_timer_get_time();
    if ((currTriggerTime - lastTriggerTime) > 6000)
    {
        esp_timer_stop(singleShot_timerOn);

        // swap tables used
        triggerTableType* temp = g_activeTable;
        g_activeTable = g_preparingTable;
        g_preparingTable = temp;

        uint64_t deltaTime_us = g_activeTable[0].deltaDelay_us;
        if (deltaTime_us < (MAX_TRIGGER_DELAY))
        {
            ESP_ERROR_CHECK(esp_timer_start_once(singleShot_timerOn, deltaTime_us));
        }

        g_updateTable_flg = true;   // flag that a new table will be needed

        lastTriggerTime = currTriggerTime;
    }
}

void timer_ON_isr_handler(void* arg)
{
    uint32_t mask = g_activeTable[g_triggerCounter].mask;

    gpio_set_level(g_triggerPins[0], (mask >> 0) & 0x01);
    gpio_set_level(g_triggerPins[1], (mask >> 1) & 0x01);
    gpio_set_level(g_triggerPins[2], (mask >> 2) & 0x01);
    gpio_set_level(g_triggerPins[3], (mask >> 3) & 0x01);
    gpio_set_level(g_triggerPins[4], (mask >> 4) & 0x01);
    gpio_set_level(g_triggerPins[5], (mask >> 5) & 0x01);
    gpio_set_level(g_triggerPins[6], (mask >> 6) & 0x01);
    gpio_set_level(g_triggerPins[7], (mask >> 7) & 0x01);
    gpio_set_level(g_triggerPins[8], (mask >> 8) & 0x01);
    gpio_set_level(g_triggerPins[9], (mask >> 9) & 0x01);


    g_triggerCounter++;
    // reloads timer_ON_isr_handler if another triggering needs to happen before the next cycle
    if ((MAX_TRIGGER_DELAY > g_activeTable[g_triggerCounter].delay_us) &&
        (0 < g_activeTable[g_triggerCounter].deltaDelay_us) &&
        (g_triggerCounter < (NUM_OF_TRIGGERS + 1)))
    {
        uint64_t deltaTime_us = g_activeTable[g_triggerCounter].deltaDelay_us;
        ESP_ERROR_CHECK(esp_timer_start_once(singleShot_timerOn, deltaTime_us));
    }
    else
    {
        // reset the timer when the last operation was completed
        g_triggerCounter = 0;
    }
}

extern "C" void app_main()
{
    // Initialize UART0
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_0, &uart_config);
    uart_driver_install(UART_NUM_0, 1024 * 2, 0, 0, NULL, 0);

    {
        logicConfigType logicCfg;
        logicCfg.numOfTriggers = NUM_OF_TRIGGERS;
        init_logic(&logicCfg);
    }

    {
        config_duty_cycle(0, 0.0f, 0);
        config_duty_cycle(1, 0.0f, 0);
        config_duty_cycle(2, 0.0f, 0);
        config_duty_cycle(3, 0.0f, 0);
        config_duty_cycle(4, 0.0f, 0);
        config_duty_cycle(5, 0.0f, 0);
        config_duty_cycle(6, 0.0f, 0);
        config_duty_cycle(7, 0.0f, 0);
        config_duty_cycle(8, 0.0f, 0);
        config_duty_cycle(9, 0.0f, 0);
    }

    {
        const esp_timer_create_args_t on_timer_args = {
            .callback = &timer_ON_isr_handler,
            .arg = NULL,
            .dispatch_method = ESP_TIMER_TASK,
            .name = "On",
        };
        ESP_ERROR_CHECK(esp_timer_create(&on_timer_args, &singleShot_timerOn));

        const esp_timer_create_args_t delay_timer_args = {
            .callback = &timer_DELAY_isr_handler,
            .arg = NULL,
            .dispatch_method = ESP_TIMER_TASK,
            .name = "Delay",
        };
        ESP_ERROR_CHECK(esp_timer_create(&delay_timer_args, &singleShot_timerDelay));
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

        for (int i = 0; i < NUM_OF_TRIGGERS; i++)
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
        gpio_isr_handler_add(GPIO_NUM_34, gpio_isr_handler, (void*) GPIO_NUM_34); // add the custom ISR handler
    }

    ComCan_init();

    while(true)
    {
        if (true == g_updateTable_flg)
        {
            calc_duty_cycle();
            calc_new_table(g_preparingTable);
        }

        ComCan_receive();
        canRxDataType* rxDataPtr = ComCan_get_rxData();
        if (1 == rxDataPtr->dataReceived)
        {
            if (1001 > rxDataPtr->dutyCycleReq)
            {
                for (int i = 0; i < NUM_OF_TRIGGERS; i++)
                {
                    if ((rxDataPtr->triggerIndexMask >> i) & 0x01)
                    {
                        float endPrcnt = ((float) rxDataPtr->dutyCycleReq) / 1000.0f;
                        config_duty_cycle(i, endPrcnt, rxDataPtr->timeRequest_10ms);
                    }
                }
            }
            rxDataPtr->dataReceived = 0;
        }

        static uint32_t counter = 0;
        if (counter > 100)
        {
            float* dutyCyclesPtr = logic_get_duty_cycles();
            ComCan_transmit(dutyCyclesPtr, NUM_OF_TRIGGERS);
            counter = 0;
        }
        counter += 5;

        vTaskDelay(5 / portTICK_PERIOD_MS);
    }
}