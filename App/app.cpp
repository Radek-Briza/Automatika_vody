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
#include  "DataTransmit.hpp"
#include "Message.hpp"
#include "WdtSystemTask.hpp"
#include "Common.hpp"
#include "PumpControl.hpp"
#include "TaskPriorities.hpp" 


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
        REQUEST_SENDER_TASK_PRIOR,
        nullptr);
    configASSERT(Create1 == pdPASS);

   /* create task for receive data answer  */ 
 	auto Create2 = xTaskCreate(
        ResponseHandlerTask, 
        "Response Handler",
        512,
        nullptr,
        RESPONSE_HANDLER_TASK_PRIOR, 
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
		const TickType_t period =pdMS_TO_TICKS(PUMP_CONTROL_TASK_INTERVAL);
		TickType_t lastWake = xTaskGetTickCount();
		
		PumpControler::GetInstance().ControlPump();
		vTaskDelayUntil(&lastWake, period);
		gAliveMask.fetch_or(TASK_PUMP_BIT);  

	}
}

/* init pump driver  */
void InitPumpSystem(void){
  	auto ErrLed = [](bool on) { 
		on ? BSP_LED_On(LED_RED) : BSP_LED_Off(LED_RED);
	};
	auto RunLed = [](bool on) { 
		 on ? BSP_LED_On(LED_BLUE) : BSP_LED_Off(LED_BLUE);
	};
	auto RunPin = [](bool on) { 
		on ? HAL_GPIO_WritePin(PUMP_CONTROL_GPIO_Port,PUMP_CONTROL_Pin, GPIO_PIN_SET): 
		     HAL_GPIO_WritePin(PUMP_CONTROL_GPIO_Port,PUMP_CONTROL_Pin, GPIO_PIN_RESET);
	};
	PumpControler::GetInstance().Init(QueuePumpControl,ErrLed,RunLed,RunPin) ;
	
	auto ok =  xTaskCreate(
        PumpControlTask,
        "Pump control",
        256,
        nullptr,
        PUMP_CONTROL_TASK_PRIOR, 
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
		const TickType_t period =pdMS_TO_TICKS(POLL_NEW_DATA_PERIOD_MS);
		TickType_t lastWake = xTaskGetTickCount();

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
					pdMS_TO_TICKS(100)
				);
				configASSERT(ok  == pdPASS) ;		
			}
			BSP_LED_Toggle(LED_GREEN);
			vTaskDelay(pdMS_TO_TICKS(100));
			BSP_LED_Toggle(LED_GREEN);
		
		} /* communication error */
		else if(DataTransmit::GetInstance().SlaveNotResponding){
			#if APP_DEBUG_PRINT
			printf("Slave not responding! No response received within timeout period.\n");
			#endif
			DataTransmit::GetInstance().SlaveNotResponding = false; // Reset flag after handling no response
		
			/* display - send error state  */
				msgDisplay.MsgType = MsgDataType::CommunicationError;
				msgDisplay.Data = 1; 
				auto ok = xQueueSend(
            	QueueDisplay,
            	&msgDisplay ,
            	pdMS_TO_TICKS(100)
        		);
				configASSERT(ok  == pdPASS) ;
		}
		vTaskDelayUntil(&lastWake, period);
		gAliveMask.fetch_or(TASK_APP_BIT);	
	}
}