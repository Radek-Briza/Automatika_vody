
#ifndef MESSAGE_HPP_
#define  MESSAGE_HPP_

#include <cstdint>
#include "queue.h"

const uint32_t QueueLength = 10;

enum class MsgDataType {
    LevelData,
    LevelStatusData,
    BatteryLevel,
    PumpError,
    CommunicationError,
    UnknownDataType
};

struct Message
{
    MsgDataType  MsgType=MsgDataType::UnknownDataType;;
    uint32_t MsgCounter=0;
    uint32_t Data=0;
};



 extern QueueHandle_t QueuePumpControl;
 extern QueueHandle_t QueueDisplay;
 extern QueueHandle_t QueueLog;

 #endif