#pragma once

#include "main.h"  // IWYU pragma: keep.
#include "FreeRTOS.h" // IWYU pragma: keep.
#include "Message.hpp" // IWYU pragma: keep.


constexpr TickType_t POLL_PERIOD_MS       = 10;
constexpr TickType_t DEBOUNCE_PRESS_MS    = 100;
constexpr TickType_t DEBOUNCE_RELEASE_MS  = 100;
constexpr TickType_t LONG_PRESS_MS        = 1500;
constexpr uint8_t BUTTON_COUNT = 2;


enum class ButtonState{
    Idle,
    DebouncePress,
    Pressed,
    LongPressed,
    DebounceRelease
};

struct ButtonContext{
    GPIO_TypeDef* port;
    uint16_t pin;
    ButtonState state = ButtonState::Idle;
    TickType_t timestamp = 0;
    bool longPressSent = false;
};


void ButtonControlInit(void);
