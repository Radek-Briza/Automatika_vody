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
#include "Common.hpp"
#include "PumpControl.hpp"


	
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
	
	/* create task for periodic req send */
 	auto  Create1 = xTaskCreate(
        RequestSendTask, 
        "RequestSend",
        256,
        nullptr,
        2,
        nullptr);
    configASSERT(Create1 == pdPASS);

   /* create task for receive data answer  */ 
 	auto Create2 = xTaskCreate(
        ResponseHandlerTask, 
        "Response Handler",
        512,
        nullptr,
        2,
        nullptr);
    configASSERT(Create2 == pdPASS);

	#if APP_DEBUG_PRINT
	printf("Init device data  transmit\r"); 
	#endif
}

/* pump control task */
[[maybe_unused]] 
void PumpControlTask(void* argument){ 
	while (1){
		PumpControler::GetInstance().ControlPump();
		gAliveMask.fetch_or(TASK_PUMP_BIT);  
	}
}


void InitPumpSystem(void){
  	auto ErrLed = [](bool on) { 
		if(on){BSP_LED_On(LED_RED);}
			else {BSP_LED_Off(LED_RED);}
	};
	auto RunLed = [](bool on) { 
		if(on){BSP_LED_On(LED_GREEN);}
			else {BSP_LED_Off(LED_GREEN);}
	};
	auto RunPin = [](bool on) { 
		if(on){BSP_LED_On(LED_BLUE);}
			else {BSP_LED_Off(LED_BLUE);} 
	};
	PumpControler::GetInstance().Init(QueuePumpControl,ErrLed,RunLed,RunPin) ;
	
	auto ok =  xTaskCreate(
        PumpControlTask,
        "Pump control",
        256,
        nullptr,
        2,
        nullptr);
    
      configASSERT(ok == pdPASS);	  
} 

/* transmit request task */
[[maybe_unused]] 
void RequestSendTask(void* argument){ 
    while (true){
       vTaskDelay(pdMS_TO_TICKS(REQ_SEND_INTERVAL));
	   BSP_LED_Toggle(LED_BLUE);
	   DataTransmit::GetInstance().SendRquest(Packet::Level_request);
	   BSP_LED_Toggle(LED_BLUE);
	   gAliveMask.fetch_or(TASK_REQ_SENDER_BIT);
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
			#if APP_DEBUG_PRINT
			printf("Data received successfully, type: %d\n", DataTransmit::GetInstance().GetReceivedDataType());
			#endif
			DataTransmit::GetInstance().DataAvailable = false; // Reset flag after processing
			if(DataTransmit::GetInstance().GetReceivedDataType()==Packet::Level_response){
				uint32_t level_value = ParsePayload<uint16_t>( DataTransmit::GetInstance().GetReceivedPayload(), 0);
				uint32_t  level_status = ParsePayload<uint16_t>( DataTransmit::GetInstance().GetReceivedPayload(), 1); 
				configASSERT( level_value not_eq 0xFFFFFFFF or level_status not_eq 0xFFFFFFFF);  
			
				/* display */
				msgDisplay.MsgType = MsgDataType::LevelData;
				msgDisplay.Data = level_value; 
				auto ok = xQueueSend(
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
			#if APP_DEBUG_PRINT
			printf("Data overload! New data received before processing previous data.\n");
			#endif
			DataTransmit::GetInstance().DataOverload = false; // Reset flag after handling overload
			BSP_LED_Toggle(LED_RED);
			 vTaskDelay(pdMS_TO_TICKS(100));
			BSP_LED_Toggle(LED_RED);
		}
		else if(DataTransmit::GetInstance().SlaveNotResponding){
			#if APP_DEBUG_PRINT
			printf("Slave not responding! No response received within timeout period.\n");
			#endif
			DataTransmit::GetInstance().SlaveNotResponding = false; // Reset flag after handling no response
			BSP_LED_Toggle(LED_RED);
			vTaskDelay(pdMS_TO_TICKS(100));
			BSP_LED_Toggle(LED_RED);
			
			/* display - sen error state  */
				msgDisplay.MsgType = MsgDataType::CommunicationError;
				msgDisplay.Data = 1; 
				auto ok = xQueueSend(
            	QueueDisplay,
            	&msgDisplay ,
            	pdMS_TO_TICKS(300)
        		);
				configASSERT(ok  == pdPASS) ;
		}
		gAliveMask.fetch_or(TASK_APP_BIT);
	}
}

