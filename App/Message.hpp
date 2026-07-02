#pragma once 

#include <cstdint>
#include "FreeRTOS.h" // IWYU pragma: keep.
#include "queue.h"

const uint32_t QueueLength = 10;

enum class MsgDataType {
    LevelData,
    LevelStatusData,
    BatteryLevel,
    PumpStatus,
    PumpError,
    CommunicationError,
    AtomatikaOff,
    MaxLevel,
    UnknownDataType
};

struct Message{
    MsgDataType  MsgType=MsgDataType::UnknownDataType;;
    uint32_t MsgCounter=0;
    uint32_t Data=0;
};

enum class ButtonEventType{
    Press,
    LongPress,
    Release,
};

struct MessageButton{
    uint8_t buttonId;
    ButtonEventType event;
};

 extern QueueHandle_t QueuePumpControl;
 extern QueueHandle_t QueueDisplay;
 extern QueueHandle_t QueueLog;
 extern QueueHandle_t gButtonQueue;

 