

#pragma once

#include <array>
#include <atomic>
#include <functional>
#include <cstdint>

#include "FreeRTOS.h"
#include "task.h"

class LedController{
public:

    enum class Leds : uint8_t{
        ErrorLed = 0,
        CommunicationLed,
        PumpOnLed,
        Buzzer,
        AutomatOnLed,
        Count
    };

    enum class LedMode : uint8_t{
        Off,
        On,
        Blink,
        OneShot
    };

    struct LedState{
    std::atomic<LedMode> mode;
    std::atomic<TaskHandle_t> owner;
    };

    using LedCallback = std::function<void(bool)>;

    static void Init(
        LedCallback red,
        LedCallback green,
        LedCallback blue,
        LedCallback buzzer,
        LedCallback AutomatOn
    );

    static bool Acquire(Leds led);
  
    static void Release(Leds led);

    static void SetMode(
        Leds led,
        LedMode mode);

    static LedMode GetMode(
        Leds led);

    static void Task();

private:
        static size_t ToIndex(Leds led){
            const auto index = static_cast<size_t>(led);
            configASSERT(index < LED_COUNT);
            return index;
        }

    static constexpr TickType_t PERIOD = pdMS_TO_TICKS(100);

    static constexpr size_t LED_COUNT = static_cast<size_t>(Leds::Count);

    static std::array<LedState, LED_COUNT> leds_;

    static std::array<LedCallback,LED_COUNT> callbacks_;
};


void LedDriverInit();