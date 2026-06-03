#pragma once 

#include "Message.hpp" 
#include "timers.h"
#include <functional>
//#include <type_traits>
#include "LedController.hpp"

class PumpControler {
    public:
        static PumpControler & GetInstance() {
            static PumpControler instance;
            return instance;
        }
        static const uint32_t LEVEL_L = (1u << 0);
        static const uint32_t LEVEL_UNDER_M = (1u << 1);
        static const uint32_t LEVEL_H = (1u << 2);
        static const uint32_t PUMP_MAX_RUN_TIME  =  6000U;

        enum class DisplayMessageType{
            NoMessage,
            PumpRun,
            PumpStop,
        };
     using LedHandler = std::function<void(bool)>; 
            
        void Init(QueueHandle_t &QueuePumpControl_,
                LedHandler PumpControlPin_ );
        void ControlPump();
        inline void ClearErrorState(){ErrorCondition = false; LedController::SetMode(LedController::Leds::Red,LedController::LedMode::Off); };

    private:
        PumpControler() = default;
        ~PumpControler() = default;
        PumpControler(const PumpControler&) = delete;
        PumpControler& operator=(const PumpControler&) = delete;
        PumpControler(PumpControler&&) = delete;
        PumpControler& operator=(PumpControler&&) = delete;
        
        //static std::function<void(bool)> ErrorLedControl;
        //static std::function<void(bool)> RunLedControl;
        static std::function<void(bool)> PumpControlPin;
        static bool PumpRun ;
        static bool ErrorCondition ;
        static TimerHandle_t PumpRunTimer; 
        static QueueHandle_t QueuePumpControl;
        static Message msgDisplay;
        static Message msgPumpControl;

        friend void PumpOvertimerCallback(TimerHandle_t xTimer);
};
