#include "main.h"
#include "FreeRTOS.h" // IWYU pragma: keep.
#include "task.h"
#include "Message.hpp"
#include <cstdio>
#include "SSD1306_OLED.hpp"

#define myOLEDwidth  128
#define myOLEDheight 64
#define FULLSCREEN (myOLEDwidth * (myOLEDheight/8))

/* bitova maska  priznaku napisu*/
enum class DisplayUpdate{
    LevelData = (1u << 0),
    PumpRun = (1u << 1), 
    PumpError = (1u << 2),
    CommunicationError = (1u << 3)
};



[[maybe_unused]] 
void DisplayTask(void* argument){ 
    bool CommunicationError =false;
    Message msg;
    uint32_t displayUpdate = 0;
    bool displayNeedsUpdate = false; 
    uint8_t step =0;
    
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
    
        myOLED.OLEDbegin( I2C_Address, I2C_debug); // initialize the OLED
        myOLED.OLEDFillScreen(0xF0, 0); // splash screen bars, optional just for effect
        if (!myOLED.OLEDSetBufferPtr(
            myOLEDwidth, myOLEDheight,
            screenBuffer,
            sizeof(screenBuffer))) return;
        myOLED.OLEDclearBuffer();
        myOLED.setTextColor(WHITE);
        myOLED.setTextSize(2);
        myOLED.setCursor(0, 0);
        myOLED.print("Hladina:");
        myOLED.OLEDupdate();
    }

    while (true){  
        auto ok= xQueueReceive(
                QueueDisplay,
                &msg,
                50
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
                displayUpdate |= static_cast<uint32_t>(DisplayUpdate::LevelData);
                displayUpdate &= ~static_cast<uint32_t>(DisplayUpdate::CommunicationError);
                displayNeedsUpdate = true;
                        
            }
            /* communication error rise */
             if(msg.MsgType == MsgDataType::CommunicationError && msg.Data ==1){      
                if(! CommunicationError ){
			        printf("<< CHYBA KOMUNIKACE >>\n");
                    CommunicationError = true;
                    displayUpdate |= static_cast<uint32_t>(DisplayUpdate::CommunicationError);
                    displayNeedsUpdate = true;
                    }   
            }
            /* pump error rise */
            if(msg.MsgType == MsgDataType::PumpError && msg.Data ==1){  
                displayUpdate |= static_cast<uint32_t>(DisplayUpdate::PumpError);
                displayNeedsUpdate = true;      
			    printf("<< CHYBA CERPADLA >>\n");
            }
            /* pump error clear */
            if(msg.MsgType == MsgDataType::PumpError && msg.Data ==0){    \
                displayUpdate = static_cast<uint32_t>(static_cast<uint32_t>(displayUpdate) & ~static_cast<uint32_t>(DisplayUpdate::PumpError));    
			    printf("<< CERPADLO ODBLOKOVANO >>\n");
            }
            /* pump start run */
             if(msg.MsgType == MsgDataType::PumpStatus && msg.Data ==1){        
			    printf("<< CERPADLO ZAPNUTO >>\n");
                displayUpdate |= static_cast<uint32_t>(DisplayUpdate::PumpRun);
                displayNeedsUpdate = true;
            }
             /* pump stop */
             if(msg.MsgType == MsgDataType::PumpStatus && msg.Data ==0){        
			    printf("<< CERPADLO  VYPNUTO >>\n");
                displayUpdate = static_cast<uint32_t>(static_cast<uint32_t>(displayUpdate) & ~static_cast<uint32_t>(DisplayUpdate::PumpRun));
                displayNeedsUpdate = true;
                 step =0;
            }
        }
        
        /* update display if needed */
        if(displayNeedsUpdate){

             /* pump error */
            if(static_cast<uint32_t>(displayUpdate) & static_cast<uint32_t>(DisplayUpdate::PumpError)){
                myOLED.OLEDclearBuffer();
                myOLED.setTextSize(2);
                myOLED.setCursor(0, 0);
                myOLED.print("CHYBA CERPADLA");
                myOLED.setTextSize(3);
                myOLED.setCursor(0, 25);
                myOLED.print(msg.Data);
                myOLED.OLEDupdate();
                continue;
            }

            /* communication error */
            if(static_cast<uint32_t>(displayUpdate) & static_cast<uint32_t>(DisplayUpdate::CommunicationError)){
               
                myOLED.OLEDclearBuffer();
                myOLED.setTextSize(2);
                myOLED.setCursor(0, 20);
                myOLED.print("CHYBA     KOMUNIKACE");
                myOLED.setTextSize(3);
                myOLED.setCursor(0, 
                    25);  
                    myOLED.OLEDupdate();
                continue;
            }

            /* level data */           
            if(static_cast<uint32_t>(displayUpdate) & static_cast<uint32_t>(DisplayUpdate::LevelData)){
                myOLED.OLEDclearBuffer();
                myOLED.setTextSize(2);
                myOLED.setCursor(0, 0);
                myOLED.print("Hladina:");
                myOLED.setTextSize(3);
                myOLED.setCursor(0, 
                    25);  
                myOLED.print(msg.Data);
                displayUpdate = static_cast<uint32_t>(static_cast<uint32_t>(displayUpdate) & ~static_cast<uint32_t>(DisplayUpdate::LevelData));
            }

            /* pumm run animation */
             if(static_cast<uint32_t>(displayUpdate) & static_cast<uint32_t>(DisplayUpdate::PumpRun)){        
                myOLED.drawCircle(80,40,15,WHITE);
                // Triangle vertices on circle with center (80,40) and radius 15
                switch (step){

                 case 0:
                  myOLED.drawTriangle(80,25, 69,51, 91,51, BLACK);
                    myOLED.drawTriangle(80,45, 69,51, 91,51, WHITE);
                    step++;
                    break;
                case 1:
                    myOLED.drawTriangle(80,45, 69,51, 91,51, BLACK);
                    myOLED.drawTriangle(80,35, 69,51, 91,51, WHITE);
                    step++;
                    break;
                case 2:
                    myOLED.drawTriangle(80,35, 69,51, 91,51, BLACK);
                    myOLED.drawTriangle(80,25, 69,51, 91,51, WHITE);
                    step =0;
                    break;    
                }
               
            }
            myOLED.OLEDupdate();
            if(displayUpdate == 0){
                displayNeedsUpdate = false;
            }
        }


    }   


}  