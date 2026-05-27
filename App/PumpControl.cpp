
#include "PumpControl.hpp"
#include "Main.h" // IWYU pragma: keep.
#include <cstdio>
#include "Common.hpp"

std::function<void(bool)> PumpControler::ErrorLedControl=nullptr;
std::function<void(bool)> PumpControler::RunLedControl=nullptr;
std::function<void(bool)> PumpControler::PumpControlPin=nullptr;
bool PumpControler::PumpRun=false ;
bool PumpControler::ErrorCondition=false ;
TimerHandle_t PumpControler::PumpRunTimer=nullptr; 
QueueHandle_t PumpControler::QueuePumpControl=nullptr;
Message PumpControler::msgDisplay={};
//MessageButton PumpControler::BtnMsg;


/* pump overtime  handler */
[[maybe_unused]] 
void PumpOvertimerCallback(TimerHandle_t xTimer){
    configASSERT(PumpControler::RunLedControl   != nullptr) ;
    configASSERT(PumpControler::PumpControlPin  != nullptr) ;
    configASSERT(PumpControler::ErrorLedControl != nullptr) ;

    if(PumpControler::PumpRun){
        PumpControler::PumpRun = false;
        PumpControler::ErrorCondition = true;
        PumpControler::RunLedControl(false);
        PumpControler::PumpControlPin(false);
        PumpControler::ErrorLedControl(true);
        #if APP_DEBUG_PRINT
        printf("Pump overtime - OFF\r\n");
        #endif
       
        /* error status for  display */
        PumpControler::msgDisplay.MsgType = MsgDataType::PumpError;
        PumpControler::msgDisplay.Data = 1;  // persist 
        auto ok = xQueueSend(
        QueueDisplay,
        & PumpControler::msgDisplay ,
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
    configASSERT(gButtonQueue != nullptr);
    DisplayMessageType NewMessage = DisplayMessageType::NoMessage;

    /* get message  from radio module */
    Message StatusMsg;
    auto ok= xQueueReceive(
                QueuePumpControl,
                &StatusMsg,
                0
            );

    /* new message - level status */
    if(ok == pdPASS){
        if(StatusMsg.MsgType == MsgDataType::LevelStatusData ){
            if(!ErrorCondition){
                switch (StatusMsg.Data & (LEVEL_L | LEVEL_UNDER_M | LEVEL_H)){
                    
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
                        NewMessage = DisplayMessageType::PumpRun;
                        #if APP_DEBUG_PRINT
                        printf("Pump ON(1)\r\n");
                        #endif
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
                     NewMessage = DisplayMessageType::PumpRun;
                    #if APP_DEBUG_PRINT
                    printf("Pump ON(2)\r\n");
                    #endif
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
                         NewMessage = DisplayMessageType::PumpStop;
                        #if APP_DEBUG_PRINT
                        printf("Pump OFF\r\n");
                        #endif
                    }          
                }
            }
        }
     }   
    
    /* send message to display if a some change */
    if( NewMessage != DisplayMessageType::NoMessage){
        PumpControler::msgDisplay.MsgType = MsgDataType::PumpStatus;
        PumpControler::msgDisplay.Data = (NewMessage == DisplayMessageType::PumpRun) ? 1 : 0;
        auto ok = xQueueSend(
        QueueDisplay,
        & PumpControler::msgDisplay ,
        pdMS_TO_TICKS(300)
        );
        configASSERT(ok  == pdPASS) ;

    }

     /* message from button - possible unblock system   */
     MessageButton BtnMsg;
     ok= xQueueReceive(
                gButtonQueue,
                &BtnMsg,
                0
            );

    /* new message - button */
    if(ok == pdPASS){
        if(BtnMsg.buttonId==1 && BtnMsg.event == ButtonEventType::LongPress){
            if(PumpControler::ErrorCondition == true){
            PumpControler::ErrorCondition = false;
            PumpControler::ErrorLedControl(false);
            /* clear error status for  display */
            msgDisplay.MsgType = MsgDataType::PumpError;
            msgDisplay.Data = 0; // clear 
            auto ok = xQueueSend(
            QueueDisplay,
            &msgDisplay ,
            pdMS_TO_TICKS(300)
            );
            configASSERT(ok  == pdPASS) ;
            }
            #if APP_DEBUG_PRINT
            printf("<<<< Cepadlo odblokovano >>>>\r\n");
            #endif
        }

    }
    
}
