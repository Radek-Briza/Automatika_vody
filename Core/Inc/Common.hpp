#ifndef COMMON_HPP_
#define COMMON_HPP_
#include "main.h"

constexpr uint32_t PARAM_STORAGE_ADDRESS = 0x0803F800;

#define REQ_SEND_INTERVAL          9000u
#define POLL_NEW_DATA_PERIOD_MS     10u
#define PUMP_CONTROL_TASK_INTERVAL  10u
#define  BATTERY_REQUEST_INTERVAL      10u

// debug print switches 
#define  RADIO_DEBUG_PRINT      0
#define  APP_DEBUG_PRINT        1
#define  DISPLAY_MODULE_ENABLE  1

#define WDT_ENABLE          0

#endif