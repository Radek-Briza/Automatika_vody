
#include "FreeRTOS.h" // IWYU pragma: keep.
#include "task.h"
#include "Message.hpp"
#include <cstdio>
#include "timers.h"
#include "PumpControl.hpp"
#include "WdtSystemTask.hpp"

constexpr uint32_t LEVEL_L = (1u << 0);
constexpr uint32_t LEVEL_UNDER_M = (1u << 1);
constexpr uint32_t LEVEL_H = (1u << 2);
constexpr uint32_t PUMP_MAX_RUN_TIME  =  30000U;



static TimerHandle_t PumpRunTimer = nullptr;
 bool PumpRun = false ;
 bool ErrorCondition = false;

[[maybe_unused]] 
void PumpOvertimerCallback(TimerHandle_t xTimer)
{
    if(PumpRun){
        PumpRun = false;
        ErrorCondition = true;
        printf("Pump overtime - OFF\r\n");
    }
}


void InitPumpSystem(   void){

    auto Create3 =  xTaskCreate(
        PumpControlTask,
        "Pump control",
        256,
        nullptr,
        2,
        nullptr);
    
      configASSERT(Create3 == pdPASS);


    PumpRunTimer = xTimerCreate(
        "Pump timer",
        pdMS_TO_TICKS(PUMP_MAX_RUN_TIME ),
        pdFALSE,
        nullptr,
        PumpOvertimerCallback);
    	configASSERT(PumpRunTimer != nullptr);
}


[[maybe_unused]] 
void PumpControlTask(void* argument){ 
     BaseType_t ok;
     Message msg;

    while (true) {
        ok= xQueueReceive(
                QueuePumpControl,
                &msg,
                0
            );

            /* new message */
        if(ok == pdPASS){
            if(msg.MsgType == MsgDataType::LevelStatusData ){
                if(!ErrorCondition){

                    switch (msg.Data & (LEVEL_L | LEVEL_UNDER_M | LEVEL_H)){
                        
                        /* under max and middle - hysteresis */
                        case 0:
                        break;

                        /* under max , turn ON pump*/
                        case LEVEL_L:
                        if(! PumpRun){
                            PumpRun = true;
                    
                            auto Ok = xTimerStart(PumpRunTimer,0U);
		                    configASSERT(Ok == pdPASS);
                            // gpio pump on 
                            printf("Pump ON(1)\r\n");
                        }
                        break;
                    /* drop level  */
                    case LEVEL_UNDER_M:
                    if(!PumpRun ){
                            PumpRun = true;
                            auto Ok = xTimerStart(PumpRunTimer,0U);
		                    configASSERT(Ok == pdPASS);
                            // gpio pump on 
                            printf("Pump ON(2)\r\n");
                        }
                      
                    break;    
                
                    /* max limit , turn off pump  */
                    case LEVEL_H:
                   default:
                        if(PumpRun){
                            PumpRun = false;
                             auto Ok = xTimerStop(PumpRunTimer,0U);
		                    configASSERT(Ok == pdPASS);
                            // gpio pump on 
                            printf("Pump OFF\r\n");
                        }
                
                    }
                }
            }
        }
     } 
     gAliveMask.fetch_or(TASK_PUMP_BIT);   
}