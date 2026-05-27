#include "FreeRTOS.h" // IWYU pragma: keep.
#include "task.h"
#include "semphr.h"
#include <cstdio>
#include <atomic>
#include "iwdg.h"
#include "WdtSystemTask.hpp"
#include "Common.hpp"


extern IWDG_HandleTypeDef hiwdg;


std::atomic<uint32_t> gAliveMask{0};

constexpr uint32_t EXPECTED_MASK = TASK_PUMP_BIT | TASK_APP_BIT | TASK_REQ_SENDER_BIT | TASK_BTN_DRIVER_BIT;

[[maybe_unused]] 
void WdtSupervisorTask(void*){
    MX_IWDG_Init();

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(REQ_SEND_INTERVAL));
        uint32_t snapshot = gAliveMask.exchange(0);

        if (snapshot == EXPECTED_MASK){         
            HAL_IWDG_Refresh( &hiwdg);
        }
    }
}
