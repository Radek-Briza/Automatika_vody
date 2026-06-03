
#
#include "LedController.hpp"

#include "FreeRTOS.h"
#include "task.h"

#include "main.h"

std::array<LedController::LedState, LedController::LED_COUNT> LedController::leds_;

std::array<LedController::LedCallback,3> LedController::callbacks_;

void LedController::Init(
    LedCallback led0,
    LedCallback led1,
    LedCallback led2){
    
    callbacks_[0] = led0;
    callbacks_[1] = led1;
    callbacks_[2] = led2;

    leds_[0].mode.store(
        LedMode::Off,
        std::memory_order_relaxed);
    leds_[0].owner.store(nullptr, std::memory_order_relaxed);

    leds_[1].mode.store(
        LedMode::Off,
        std::memory_order_relaxed);
    leds_[1].owner.store(nullptr, std::memory_order_relaxed);

    leds_[2].mode.store(
        LedMode::Off,
        std::memory_order_relaxed);
    leds_[2].owner.store(nullptr, std::memory_order_relaxed);       
}


bool LedController::Acquire(Leds led){
    auto index = ToIndex(led); 

    TaskHandle_t me =
        xTaskGetCurrentTaskHandle();
    if (leds_[index].owner.load() == nullptr){
        leds_[index].owner.store(me, std::memory_order_relaxed);
        return true;
    }
    return false;
}

void LedController::Release(Leds led) {
    auto index = ToIndex(led);

    TaskHandle_t me =
        xTaskGetCurrentTaskHandle();

    if (leds_[index].owner.compare_exchange_strong(me, nullptr,std::memory_order_acq_rel)){
        leds_[index].owner.store(nullptr, std::memory_order_relaxed);

        leds_[index].mode.store(
            LedMode::Off);
    }
  
}

void LedController::SetMode(
    Leds led,
    LedMode mode){
    auto index = ToIndex(led);

    TaskHandle_t me =
        xTaskGetCurrentTaskHandle();


    /* no owner, set mode allowed */
    if(leds_[index].owner.load() == nullptr){
         if(mode !=LedMode::Off)  leds_[index].owner = me; // only set owner if not off, otherwise release is needed to change owner
            leds_[index].mode.store(
          mode,
            std::memory_order_relaxed);
    }
    /* if owned by current task, set mode allowed */
    else {
        if (leds_[index].owner.compare_exchange_strong(me, me,std::memory_order_acq_rel)) 
         {
            leds_[index].mode.store(
            mode,
            std::memory_order_relaxed);
            if(mode ==LedMode::Off) leds_[index].owner = nullptr; // release owner if set to off    
        }
    }
}

LedController::LedMode LedController::GetMode(Leds led){
    const auto index =
        static_cast<size_t>(led);

    configASSERT(index < LED_COUNT);

    return leds_[index].mode.load(
        std::memory_order_relaxed);
}


void LedController::Task(){
    TickType_t lastWake =
        xTaskGetTickCount();

    bool blinkState = false;
    bool oneShotState = false;

    for (;;)
    {
        blinkState = !blinkState;

        for (uint8_t i = 0; i < 3; i++){
            const LedMode mode =
                leds_[i].mode.load(
                    std::memory_order_relaxed);

            switch (mode)
            {
                case LedMode::Off:
                    callbacks_[i](false);
                 //   printf("LED %d OFF\r\n",i);
                    break;

                case LedMode::On:
                    callbacks_[i](true);
                    break;

                case LedMode::Blink:
                    callbacks_[i](blinkState);
                    //printf("LED %d BLINK %d\r\n",i,blinkState);
                    break;

                case LedMode::OneShot: 
                    if(oneShotState==false){
                        callbacks_[i](true);
                        oneShotState = true;
                    } else {
                        callbacks_[i](false);
                        oneShotState = false;
                        leds_[i].mode.store(
                            LedMode::Off,
                            std::memory_order_relaxed);
                        leds_[i].owner.store(nullptr, std::memory_order_relaxed);
                    }
                         
            }
        }

        vTaskDelayUntil(
            &lastWake,
            PERIOD);
    }
}

void LedTask(void*){
    LedController::Task();
}

void LedDriverInit(){

	auto RedLed = [](bool on) { 
		on ? BSP_LED_On(LED_RED) : BSP_LED_Off(LED_RED);
	};
	auto GreenLed = [](bool on) { 
		 on ? BSP_LED_On(LED_GREEN) : BSP_LED_Off(LED_GREEN);
	};
	auto  BlueLed = [](bool on) { 
		 on ? BSP_LED_On(LED_BLUE) : BSP_LED_Off(LED_BLUE);
		     HAL_GPIO_WritePin(PUMP_CONTROL_GPIO_Port,PUMP_CONTROL_Pin, GPIO_PIN_RESET);
	};
    LedController::Init(RedLed,GreenLed,BlueLed);

    xTaskCreate(
    LedTask,
    "LED",
    512,
    nullptr,
    1,
    nullptr);
}