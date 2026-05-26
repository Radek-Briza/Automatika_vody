#include "FreeRTOS.h" // IWYU pragma: keep.
#include "task.h"
#include "Message.hpp"
#include <cstdio>

[[maybe_unused]] 
void DisplayTask(void* argument){ 
    BaseType_t ok;
    Message msg;
     while (true){  
     ok= xQueueReceive(
                QueueDisplay,
                &msg,
                portMAX_DELAY
            );

            /* new message */
        if(ok == pdPASS){
            /* level */
            if(msg.MsgType == MsgDataType::LevelData ){        
			    printf("Actual level value: %u cm\n", static_cast<unsigned int>(msg.Data));
            }
            /* pump error  */
            if(msg.MsgType == MsgDataType::PumpError ){        
			    printf("!!! CHYBA CERPADLA !!!!\n");
            
            }
        }
    }   
}  