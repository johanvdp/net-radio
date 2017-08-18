// The author disclaims copyright to this source code.
#ifndef _BLINK_H_
#define _BLINK_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"

void blink_log_configuration();
void blink_task(void *pvUnused);

#endif
