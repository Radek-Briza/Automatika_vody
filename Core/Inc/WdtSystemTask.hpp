#include <atomic>

void WdtSupervisorTask(void*);

extern  std::atomic<uint32_t> gAliveMask;

constexpr uint32_t TASK_PUMP_BIT = (1u << 0);
constexpr uint32_t TASK_APP_BIT = (1u << 1);
