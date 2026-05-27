
#ifndef PUMPCONTROLER_HPP_
#define PUMPCONTROLER_HPP_

#include "Message.hpp" 
#include "timers.h"
#include <functional>
#include <type_traits>

class PumpControler {
    public:
        static PumpControler & GetInstance() {
            static PumpControler instance;
            return instance;
        }
        static const uint32_t LEVEL_L = (1u << 0);
        static const uint32_t LEVEL_UNDER_M = (1u << 1);
        static const uint32_t LEVEL_H = (1u << 2);
        static const uint32_t PUMP_MAX_RUN_TIME  =  30000U;

        enum class DisplayMessageType{
            NoMessage,
            PumpRun,
            PumpStop,
        };
    
            
        void Init(QueueHandle_t &QueuePumpControl_,
                std::function<void(bool)> ErrorLedControl_,
                std::function<void(bool)> RunLedControl_,
                std::function<void(bool)> PumpControlPin_ );
        void ControlPump();
        inline void ClearErrorState(){ErrorCondition = false;ErrorLedControl(false); };

    private:
        PumpControler() = default;
        ~PumpControler() = default;
        PumpControler(const PumpControler&) = delete;
        PumpControler& operator=(const PumpControler&) = delete;
        PumpControler(PumpControler&&) = delete;
        PumpControler& operator=(PumpControler&&) = delete;
        
        static std::function<void(bool)> ErrorLedControl;
        static std::function<void(bool)> RunLedControl;
        static std::function<void(bool)> PumpControlPin;
        static bool PumpRun ;
        static bool ErrorCondition ;
        static TimerHandle_t PumpRunTimer; 
        static QueueHandle_t QueuePumpControl;
        static Message msgDisplay;

        friend void PumpOvertimerCallback(TimerHandle_t xTimer);
};

#endif 