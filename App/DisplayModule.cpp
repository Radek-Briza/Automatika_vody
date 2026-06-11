#include "main.h"
#include "FreeRTOS.h" // IWYU pragma: keep.
#include "task.h"
#include "Message.hpp"
#include <cstdio>
#include "SSD1306_OLED.hpp"

#define myOLEDwidth  128
#define myOLEDheight 64
#define FULLSCREEN (myOLEDwidth * (myOLEDheight/8))



[[maybe_unused]] 
void DisplayTask(void* argument){ 

    
    
    bool CommunicationError =false;
    Message msg;
    
    
    //oled initialization 
    SSD1306 myOLED(myOLEDwidth ,myOLEDheight) ; // instantiate a OLED object
    uint8_t  screenBuffer[FULLSCREEN];
    const uint8_t I2C_Address = 0x3C;
    bool I2C_debug = true;
    printf("\n\rOLED Test Begin\r\n");
    // Turn on I2C bus (optionally it may already be on)
    if(!myOLED.OLED_I2C_ON()){
	    printf("Cannot start I2C\n");
	
    }
    else {
       
        HAL_Delay(100);
        myOLED.OLEDbegin( I2C_Address, I2C_debug); // initialize the OLED
        myOLED.OLEDFillScreen(0xF0, 0); // splash screen bars, optional just for effect
        if (!myOLED.OLEDSetBufferPtr(
            myOLEDwidth, myOLEDheight,
            screenBuffer,
            sizeof(screenBuffer))) return;
        myOLED.OLEDclearBuffer();
        myOLED.setTextColor(WHITE);
        myOLED.setCursor(10, 10);
        myOLED.print("Hello World.");
        myOLED.OLEDupdate();
    }



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