
#include "PumpControl.hpp"
#include "Main.h" // IWYU pragma: keep.
#include <cstdio>

std::function<void(bool)> PumpControler::ErrorLedControl=nullptr;
std::function<void(bool)> PumpControler::RunLedControl=nullptr;
std::function<void(bool)> PumpControler::PumpControlPin=nullptr;
bool PumpControler::PumpRun=false ;
bool PumpControler::ErrorCondition=false ;
TimerHandle_t PumpControler::PumpRunTimer=nullptr; 
QueueHandle_t PumpControler::QueuePumpControl=nullptr;
Message PumpControler::InMsg;


[[maybe_unused]] 
void PumpOvertimerCallback(TimerHandle_t xTimer)
{
    static  Message msgDisplay;
    configASSERT(PumpControler::RunLedControl   != nullptr) ;
    configASSERT(PumpControler::PumpControlPin  != nullptr) ;
    configASSERT(PumpControler::ErrorLedControl != nullptr) ;

    if(PumpControler::PumpRun){
        PumpControler::PumpRun = false;
        PumpControler::ErrorCondition = true;
        PumpControler::RunLedControl(false);
        PumpControler::PumpControlPin(false);
        PumpControler::ErrorLedControl(true);

        //BSP_LED_On(LED_RED); 
        printf("Pump overtime - OFF\r\n");
        /* error status for  display */
        msgDisplay.MsgType = MsgDataType::PumpError;
        msgDisplay.Data = 0; 
        auto ok = xQueueSend(
        QueueDisplay,
        &msgDisplay ,
        pdMS_TO_TICKS(300)
        );
        configASSERT(ok  == pdPASS) ;
    }
}

void PumpControler::Init(QueueHandle_t &QueuePumpControl_,
                         std::function<void(bool)> ErrorledControl_,
                         std::function<void(bool)> RunLedControl_,
                         std::function<void(bool)> PumpControlPin_ ){

        ErrorLedControl = std::move(ErrorledControl_);
        RunLedControl = std::move(RunLedControl_);    
        PumpControlPin = std::move(PumpControlPin_);
        QueuePumpControl = QueuePumpControl_;

        /* timer init */
        PumpRunTimer = xTimerCreate(
        "Pump timer",
        pdMS_TO_TICKS(PUMP_MAX_RUN_TIME ),
        pdFALSE,
        nullptr,
        PumpOvertimerCallback);
    	configASSERT(PumpRunTimer != nullptr);
}

void PumpControler::ControlPump(){
    configASSERT(PumpControler::RunLedControl   != nullptr) ;
    configASSERT(PumpControler::PumpControlPin  != nullptr) ;
    configASSERT(PumpControler::ErrorLedControl != nullptr) ;

    auto ok= xQueueReceive(
                QueuePumpControl,
                &InMsg,
                0
            );

    /* new message */
    if(ok == pdPASS){
        if(InMsg.MsgType == MsgDataType::LevelStatusData ){
            if(!ErrorCondition){

                switch (InMsg.Data & (LEVEL_L | LEVEL_UNDER_M | LEVEL_H)){
                    
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
                        PumpControlPin(true);
                        RunLedControl(true);
                    
                        printf("Pump ON(1)\r\n");
                    }
                    break;
                /* drop level  */
                case LEVEL_UNDER_M:
                if(!PumpRun ){
                    PumpRun = true;
                    auto Ok = xTimerStart(PumpRunTimer,0U);
                    configASSERT(Ok == pdPASS);
                    RunLedControl(true);
                    PumpControlPin(true);
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
                        RunLedControl(false);
                        PumpControlPin(false);
                        printf("Pump OFF\r\n");
                    }          
                }
            }
        }
     }    
}
