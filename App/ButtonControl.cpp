#include "ButtonControl.hpp"
#include "timers.h"
#include "Common.hpp"
#include "TaskPriorities.hpp" 
#include "WdtSystemTask.hpp"

QueueHandle_t gButtonQueue = nullptr;


void SendButtonEvent(uint8_t id, ButtonEventType evt){
    id+=1;
    MessageButton msg{
        .buttonId = id,
        .event = evt
    };

    xQueueSend(
        gButtonQueue,
        &msg,
        0);
        #if APP_DEBUG_PRINT
        printf("Send  button event %d for button %d\r\n",static_cast<int>(evt) ,static_cast<int>(id));
        #endif 
}

ButtonContext gButtons[3] ={
    { BT_1_GPIO_Port, BT_1_Pin },
    { BT_2_GPIO_Port, BT_2_Pin },
    { BT_3_GPIO_Port, BT_3_Pin }
};

bool IsButtonPressed(const ButtonContext& btn){
    return HAL_GPIO_ReadPin(btn.port, btn.pin) == GPIO_PIN_RESET;
}

template<typename ReadFunc>
void ProcessButton(
    ButtonContext& btn,
    uint8_t id,
    ReadFunc&& readButton){
    const TickType_t now = xTaskGetTickCount();
    const bool pressed = readButton(btn);

    switch (btn.state){
        case ButtonState::Idle:{
            if (pressed){
                btn.timestamp = now;
                btn.state = ButtonState::DebouncePress;
            }
            break;
        }

        case ButtonState::DebouncePress:{
            if (!pressed){
                btn.state = ButtonState::Idle;
            }
            else if ((now - btn.timestamp) >= pdMS_TO_TICKS(DEBOUNCE_PRESS_MS)){
                SendButtonEvent(id, ButtonEventType::Press);
                btn.longPressSent = false;
                btn.state = ButtonState::Pressed;
            }
            break;
        }

        case ButtonState::Pressed:{
            if (!pressed){
                btn.timestamp = now;
                btn.state = ButtonState::DebounceRelease;
            }
            else if (!btn.longPressSent &&
                     ((now - btn.timestamp) >= pdMS_TO_TICKS(LONG_PRESS_MS))){
                SendButtonEvent(id, ButtonEventType::LongPress);

                btn.longPressSent = true;
                btn.state = ButtonState::LongPressed;
            }
            break;
        }

        case ButtonState::LongPressed:{
            if (!pressed){
                btn.timestamp = now;
                btn.state = ButtonState::DebounceRelease;
            }
            break;
        }

        case ButtonState::DebounceRelease:{
            if (pressed){
                btn.state = btn.longPressSent
                    ? ButtonState::LongPressed
                    : ButtonState::Pressed;
            }
            else if ((now - btn.timestamp) >= pdMS_TO_TICKS(DEBOUNCE_RELEASE_MS)){
                SendButtonEvent(id, ButtonEventType::Release);

                btn.state = ButtonState::Idle;
            }
            break;
        }
    }
}

/* task buttons monitor */
void ButtonMonitorTask(void*){
    const TickType_t period =
        pdMS_TO_TICKS(POLL_PERIOD_MS);

    TickType_t lastWake =
        xTaskGetTickCount();

    for (;;){
        for (uint8_t i = 0; i < 3; i++){
           ProcessButton(
        gButtons[i],
            i,
            [](const ButtonContext& btn) -> bool{
                return HAL_GPIO_ReadPin(btn.port, btn.pin) == GPIO_PIN_RESET;});
        }
        vTaskDelayUntil(&lastWake, period);
        gAliveMask.fetch_or(TASK_BTN_DRIVER_BIT);  
    }
}

void ButtonControlInit(void){
    
    gButtonQueue = xQueueCreate(QueueLength, sizeof(Message));
    configASSERT(gButtonQueue != nullptr);

    xTaskCreate(
    ButtonMonitorTask,
    "Buttons",
    512,
    nullptr,
    BUTTON_MONITOR_TASK_PRIOR, 
    nullptr);
}