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

#define POST_ZERO_CROSS_DETECT_DELAY_us (70u)   // delay after GPIO is triggered. The GPIO is triggered before zero-crossing actually happens

static uint32_t main_c_numOfTriggeringPins_U32 = 0;
static bool main_f_updateTable_B = true;
static int64_t main_ti_prevPeriodStart_S64 = 0;     /// Time when previous period started in us
static int64_t main_ti_currPeriodStart_S64 = 10000; /// Time when period started in us. Used to calculate period ratio (time deviation)

static uint32_t          main_c_triggerCounter_U32 = 0; // variable counts which index of the active table should be applied
static triggerTableType  main_s_triggerTable_S [2][32]; // two trigger tables - one active, one in preparation. 1 initial state and 1 state for each output
static triggerTableType* main_s_activeTable_S     = &main_s_triggerTable_S[1][0];    // active table currently being applied
static triggerTableType* main_s_preparingTable_S  = &main_s_triggerTable_S[0][0]; // table being prepared and will be used in the next cycle

static esp_timer_handle_t singleShot_initial;   ///< initial interrupt triggered by GPIO
static esp_timer_handle_t singleShot_subsqnt;   ///< subsequent interrupts triggered by timer. Sets the actual output values

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

void IRAM_ATTR apply_output(void);

void IRAM_ATTR gpio_isr_handler(void* arg)
{
    // start countdown timer as the zero-cross trigger occurs some time before actual zero-cross
    ESP_ERROR_CHECK(esp_timer_start_once(singleShot_initial, POST_ZERO_CROSS_DETECT_DELAY_us));
}

void timer_isr_handler_subsequent(void* arg)
{
    apply_output();
}

void timer_isr_handler_initial(void* arg)
{
    main_ti_prevPeriodStart_S64 = main_ti_currPeriodStart_S64;
    main_ti_currPeriodStart_S64 = esp_timer_get_time();

    // swap tables used
    triggerTableType* temp = main_s_activeTable_S;
    main_s_activeTable_S = main_s_preparingTable_S;
    main_s_preparingTable_S = temp;

    // stop active timer if it is still active
    if (esp_timer_is_active(singleShot_subsqnt))
    {
        esp_err_t err = esp_timer_stop(singleShot_subsqnt);
        if (ESP_OK != err)
        {
            ESP_LOGI("Main", "Failed to stop singleShot_subsqnt with error code %u", err);
        }
    }

    main_c_triggerCounter_U32 = 0;   // reset triggering to start from 0
    apply_output();

    // flag that a new table will be needed
    main_f_updateTable_B = true;
}

void IRAM_ATTR apply_output(void)
{
    uint32_t mask = main_s_activeTable_S[main_c_triggerCounter_U32].mask;
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

    // reloads timer if this isn't the last triggering
    if (main_s_activeTable_S[main_c_triggerCounter_U32].mask & 0x8000)   // MSB signals that this is the last triggering
    {
        // reset the timer when the last operation was completed
        main_c_triggerCounter_U32 = 0;
    }
    else
    {
        uint64_t deltaTime_us = main_s_activeTable_S[main_c_triggerCounter_U32].deltaTimeToNext_us;
        esp_err_t err = esp_timer_start_once(singleShot_subsqnt, deltaTime_us);
        main_c_triggerCounter_U32++;
        if (ESP_OK != err)
        {
            ESP_LOGI("Main", "Failed to start singleShot_subsqnt with error code %u", err);
        }
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

        // prepare triggering table
        triggerTableType tableEmptyData =
        {
            .deltaTimeToNext_us = 0,
            .triggerTime_us = 0,
            .mask = LAST_MASK_FLAG,
        };
        uint8_t numOfTables = sizeof(main_s_triggerTable_S) / sizeof(main_s_triggerTable_S[0]);
        uint8_t numOfTablesIndxs = sizeof(main_s_triggerTable_S[0]) / sizeof(main_s_triggerTable_S[0][0]);
        for (uint8_t table = 0; table < numOfTables; table++)
        {
            for (uint8_t tableIndx = 0; tableIndx < numOfTablesIndxs; tableIndx++)
            {
                main_s_triggerTable_S[table][tableIndx] = tableEmptyData;
            }
        }
    }

    // initialize interrupts
    {
        ESP_LOGI("Main", "Starting timer initialization");
        ESP_LOGI("Main", "Timer early init finished with code %u", esp_timer_early_init());
        ESP_LOGI("Main", "Timer init finished with code %u", esp_timer_init());

        const esp_timer_create_args_t on_timer_args = {
            .callback = &timer_isr_handler_subsequent,
            .arg = NULL,
            .dispatch_method = ESP_TIMER_TASK,
            .name = "timer_isr_handler_subsequent",
            .skip_unhandled_events = true,
        };
        if (ESP_OK != (esp_timer_create(&on_timer_args, &singleShot_subsqnt)))
        {
            ESP_LOGI("Main", "Failed to create singleShot_subsqnt!");
        }

        const esp_timer_create_args_t delay_timer_args = {
            .callback = &timer_isr_handler_initial,
            .arg = NULL,
            .dispatch_method = ESP_TIMER_TASK,
            .name = "timer_isr_handler_initial",
            .skip_unhandled_events = true,
        };
        if (ESP_OK != (esp_timer_create(&delay_timer_args, &singleShot_initial)))
        {
            ESP_LOGI("Main", "Failed to create singleShot_initial!");
        }

        ESP_LOGI("Main", "Finished interrupt initialization");
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
        gpio_isr_handler_add(GPIO_NUM_34, gpio_isr_handler, (void*) GPIO_NUM_34); // add the custom ISR handler
    }

    ComCan_init();

    while(true)
    {
        if (true == main_f_updateTable_B)
        {
            calc_duty_cycle();
            float periodRatio = ((float) (main_ti_currPeriodStart_S64 - main_ti_prevPeriodStart_S64)) / 10000.0f;
            calc_new_table(main_s_preparingTable_S, periodRatio);
            main_f_updateTable_B = false;
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