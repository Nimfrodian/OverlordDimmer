/*
*   ESP abstraction file
*   used to abstract away ESP related names
*
*/
#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "driver/timer.h"
#include "esp_log.h"
#include "esp_attr.h"

// Hardware abstraction
#include "tmra.h"
#include "pina.h"
#include "sera.h"