/*
 * app.cpp
 *
 *  Created on: 11. 4. 2026
 *      Author: radek
 */
#include "FreeRTOS.h" // IWYU pragma: keep.
#include "task.h"
#include  "App.hpp"
#include "radio.h"	
#include "stdio.h"
#include  "DataTransmit.hpp"
#include "Message.hpp"
#include "WdtSystemTask.hpp"

	
 QueueHandle_t QueuePumpControl = nullptr;
 QueueHandle_t QueueDisplay = nullptr;
 QueueHandle_t QueueLog = nullptr;


void InitApplication(void){
	DataTransmit::GetInstance().Init(&Radio,true); // Inicializace DataTransmitteru s nastavením MasterMode na true
	QueuePumpControl = xQueueCreate(QueueLength, sizeof(Message));
    configASSERT(QueuePumpControl != nullptr);
	QueueDisplay =  xQueueCreate(QueueLength, sizeof(Message));
	configASSERT(QueueDisplay != nullptr);
	 QueueLog =  xQueueCreate(QueueLength, sizeof(Message));
	configASSERT(QueueLog != nullptr);
	printf("Init device data  transmit\r");
}


/* transmit request task */
[[maybe_unused]] 
void RequestSendTask(void* argument){ 
    while (true){
       vTaskDelay(pdMS_TO_TICKS(5000));
	   BSP_LED_Toggle(LED_BLUE);
	   DataTransmit::GetInstance().SendRquest(Packet::Level_request);
	   BSP_LED_Toggle(LED_BLUE);
    }
}

/* response handling task */
[[maybe_unused]] 
void ResponseHandlerTask(void* argument){ 
	Message msgDisplay;
	Message msgPumpControl;

    while (true){
       /* wait for response */
		if(DataTransmit::GetInstance().DataAvailable){
			printf("Data received successfully, type: %d\n", DataTransmit::GetInstance().GetReceivedDataType());
			DataTransmit::GetInstance().DataAvailable = false; // Reset flag after processing
			if(DataTransmit::GetInstance().GetReceivedDataType()==Packet::Level_response){
				uint16_t level_value = ParsePayload<uint16_t>( DataTransmit::GetInstance().GetReceivedPayload(), 0);  
				uint16_t  level_status = ParsePayload<uint16_t>( DataTransmit::GetInstance().GetReceivedPayload(), 1);  
				BaseType_t ok;
				/* display */
				
				msgDisplay.MsgType = MsgDataType::LevelData;
				msgDisplay.Data = level_value; 
				ok = xQueueSend(
            	QueueDisplay,
            	&msgDisplay ,
            	pdMS_TO_TICKS(300)
        	);
		
			configASSERT(ok  == pdPASS) ;
			
			/* pump control  */
			msgPumpControl.MsgType = MsgDataType::LevelStatusData;
			msgPumpControl.Data = level_status;
			ok = xQueueSend(
            	QueuePumpControl,
            	&msgPumpControl ,
            	pdMS_TO_TICKS(300)
        	);
			configASSERT(ok  == pdPASS) ;
				
			}
			BSP_LED_Toggle(LED_GREEN);
			 vTaskDelay(pdMS_TO_TICKS(100));
			BSP_LED_Toggle(LED_GREEN);

		}
		else if(DataTransmit::GetInstance().DataOverload){
			printf("Data overload! New data received before processing previous data.\n");
			DataTransmit::GetInstance().DataOverload = false; // Reset flag after handling overload
			BSP_LED_Toggle(LED_RED);
			 vTaskDelay(pdMS_TO_TICKS(100));
			BSP_LED_Toggle(LED_RED);
		}
		else if(DataTransmit::GetInstance().SlaveNotResponding){
			printf("Slave not responding! No response received within timeout period.\n");
			DataTransmit::GetInstance().SlaveNotResponding = false; // Reset flag after handling no response
			BSP_LED_Toggle(LED_RED);
			vTaskDelay(pdMS_TO_TICKS(100));
			BSP_LED_Toggle(LED_RED);
		}

		 gAliveMask.fetch_or(TASK_APP_BIT);
	}
}
