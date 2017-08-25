// The author disclaims copyright to this source code.
#ifndef _BLINK_H_
#define _BLINK_H_

/**
 * @file
 * FreeRTOS task to create a blinking LED.
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"

/**
 * @brief Log configuration settings
 */
void blink_log_configuration();

/**
 * @brief The task entry function.
 * This function never returns.
 */
void blink_task(void *pvUnused);

#endif
