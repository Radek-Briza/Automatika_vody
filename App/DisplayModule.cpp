#include "FreeRTOS.h" // IWYU pragma: keep.
#include "task.h"
#include "Message.hpp"
#include <cstdio>

[[maybe_unused]] 
void DisplayTask(void* argument){ 
   bool CommunicationError =false;
    Message msg;
    
    while (true){  
        auto ok= xQueueReceive(
                QueueDisplay,
                &msg,
                portMAX_DELAY
            );

            /* new message */
        if(ok == pdPASS){
            /* level + restore communication */
            if(msg.MsgType == MsgDataType::LevelData ){        
                if(CommunicationError){
                    CommunicationError = false;
                    printf("<< KOMUNIKACE OBNOVENA >>\n");
                }
			    printf("<< AKTUALNI HLADINA: %u cm >>\n", static_cast<unsigned int>(msg.Data));
            }
            /* communication error rise */
             if(msg.MsgType == MsgDataType::CommunicationError && msg.Data ==1){      
                if(! CommunicationError ){
			        printf("<< CHYBA KOMUNIKACE >>\n");
                    CommunicationError = true;
                    }   
            }
            /* pump error rise */
            if(msg.MsgType == MsgDataType::PumpError && msg.Data ==1){        
			    printf("<< CHYBA CERPADLA >>\n");
            }
            /* pump error clear */
            if(msg.MsgType == MsgDataType::PumpError && msg.Data ==0){        
			    printf("<< CERPADLO ODBLOKOVANO >>\n");
            }
            /* pump start run */
             if(msg.MsgType == MsgDataType::PumpStatus && msg.Data ==1){        
			    printf("<< CERPADLO ZAPNUTO >>\n");
            }
             /* pump stop */
             if(msg.MsgType == MsgDataType::PumpStatus && msg.Data ==0){        
			    printf("<< CERPADLO  VYPNUTO >>\n");
            }
        }
    }   
}  