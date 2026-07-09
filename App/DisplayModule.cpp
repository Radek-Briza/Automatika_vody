#include "main.h"
#include "FreeRTOS.h" // IWYU pragma: keep.
#include "task.h"
#include "Message.hpp"
#include <cstdio>
#include "SSD1306_OLED.hpp"
#include "Common.hpp"

#define myOLEDwidth  128
#define myOLEDheight 64
#define FULLSCREEN (myOLEDwidth * (myOLEDheight/8))

/* bitova maska  priznaku napisu*/
enum class DisplayUpdate{
    LevelDataAndBattery = (1u << 0),
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
    uint8_t BatteryLevel = 0;
    int16_t  MaxLevel_value = 0;
    uint16_t  Level_value_mem = 0;
    bool PumpRunAnimation = false;
    
    //oled initialization 
    #if  DISPLAY_MODULE_ENABLE
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
    #endif

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
                displayUpdate |= static_cast<uint32_t>(DisplayUpdate::LevelDataAndBattery);
                displayUpdate &= ~static_cast<uint32_t>(DisplayUpdate::CommunicationError);
                displayNeedsUpdate = true;   
                 Level_value_mem = static_cast<uint16_t>(msg.Data);            
            }

            /* battery level */
            if(msg.MsgType == MsgDataType::BatteryLevel ){
                BatteryLevel = static_cast<uint8_t>(msg.Data);
                displayUpdate |= static_cast<uint32_t>(DisplayUpdate::LevelDataAndBattery);
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
                PumpRunAnimation = true;  
                step = 0;
            }
             /* pump stop */
             if(msg.MsgType == MsgDataType::PumpStatus && msg.Data ==0){        
			    printf("<< CERPADLO  VYPNUTO >>\n");
                displayUpdate = static_cast<uint32_t>(static_cast<uint32_t>(displayUpdate) & ~static_cast<uint32_t>(DisplayUpdate::PumpRun));
                displayNeedsUpdate = true;
                MaxLevel_value = Level_value_mem ;
                PumpRunAnimation = false;   
                 step =0;
            }
            /* automatika off */
            if(msg.MsgType == MsgDataType::AtomatikaOff && msg.Data ==0){        
			    printf("<< AUTOMATIKA VYPNUTA >>\n");
                displayUpdate = static_cast<uint32_t>(static_cast<uint32_t>(displayUpdate) & ~static_cast<uint32_t>(DisplayUpdate::PumpRun));
                displayNeedsUpdate = true;
                 step =0;
            }
        }
        
        /* update display if needed */
        #if  DISPLAY_MODULE_ENABLE
        if(displayNeedsUpdate){

             /* pump error */
            if(static_cast<uint32_t>(displayUpdate) & static_cast<uint32_t>(DisplayUpdate::PumpError)){
                myOLED.OLEDclearBuffer();
                myOLED.setTextSize(2);
                myOLED.setCursor(0, 0);
                myOLED.print("CHYBA CERPADLA");
                myOLED.setTextSize(3);
                myOLED.setCursor(0, 30);
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
            if(static_cast<uint32_t>(displayUpdate) & static_cast<uint32_t>(DisplayUpdate::LevelDataAndBattery)){
                myOLED.OLEDclearBuffer();
                myOLED.setTextSize(2);
                myOLED.setCursor(0, 0);
                myOLED.print("Hladina:");
                myOLED.setTextSize(2);
                myOLED.setCursor(0, 
                    25);  
                myOLED.print(Level_value_mem);
                myOLED.print(" cm");
                if(PumpRunAnimation == true ){
                    switch (step)
                    {
                    default:
                    case 0:
                        myOLED.print(" >");
                        step = 1;
                        break;
                    case 1:
                    myOLED.print("  >");
                        step = 2;
                        break;
                    case 2:
                    myOLED.print("   >");
                        step = 0;
                        
                    }
                } else if(Level_value_mem == MaxLevel_value && MaxLevel_value  != 0){     
                    myOLED.print(" MAX");
                    }
                myOLED.setTextSize(2);
                myOLED.setCursor(0, 50);    
                myOLED.print("Batt: ");
                myOLED.print(BatteryLevel);
                myOLED.print("%");
                displayUpdate = static_cast<uint32_t>(static_cast<uint32_t>(displayUpdate) & ~static_cast<uint32_t>(DisplayUpdate::LevelDataAndBattery));
            }
            else if(PumpRunAnimation  ){
                myOLED.OLEDclearBuffer();
                myOLED.setTextSize(2);
                myOLED.setCursor(0, 0);
                myOLED.print("Hladina:");
                myOLED.setTextSize(2);
                myOLED.setCursor(0, 25);  
                myOLED.print(Level_value_mem);
                myOLED.print(" cm");
                switch (step)
                {
                default:
                case 0:
                    myOLED.print(" >");
                    step = 1;
                    break;
                case 1:
                myOLED.print("  >");
                    step = 2;
                    break;
                case 2:
                myOLED.print("   >");
                    step = 0;
                    break;
                }           

                myOLED.setTextSize(2);
                myOLED.setCursor(0, 50);    
                myOLED.print("Batt: ");
                myOLED.print(BatteryLevel);
                myOLED.print("%");      
            }
            
            myOLED.OLEDupdate();
            if(displayUpdate == 0){
                displayNeedsUpdate = false;
            }
           
        }
         #endif
    }   
}  