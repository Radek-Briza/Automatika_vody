
#ifndef WDT_HPP_
#define WDT_HPP_

#include <atomic>

void WdtSupervisorTask(void*);

extern std::atomic<uint32_t> gAliveMask;

constexpr uint32_t TASK_PUMP_BIT = (1u << 0);
constexpr uint32_t TASK_APP_BIT = (1u << 1);
constexpr uint32_t TASK_REQ_SENDER_BIT  (1u << 2);

#endif