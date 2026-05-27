
typedef enum  {
  PriorityIdle          = 0,          
  PriorityLow           = 1,          
  PriorityNormal        = 2,          
  PriorityHigh          = 4,         
  PriorityRealtime      = 5          
} Priority;

#define  REQUEST_SENDER_TASK_PRIOR    PriorityNormal   
#define  RESPONSE_HANDLER_TASK_PRIOR  PriorityHigh 
#define  PUMP_CONTROL_TASK_PRIOR      PriorityHigh 
#define  BUTTON_MONITOR_TASK_PRIOR    PriorityNormal
#define  WDT_SUPERVISOR_TASK_PRIOR    PriorityHigh
#define  DISPLAY_TASK_PRIOR           PriorityNormal