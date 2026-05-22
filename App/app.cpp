/*
 * app.cpp
 *
 *  Created on: 11. 4. 2026
 *      Author: radek
 */
#include  "App.hpp"
#include "radio.h"	
#include "stdio.h"
#include  "DataTransmit.hpp"
extern "C"
{
#include "FreeRTOS.h" // IWYU pragma: keep.
#include "task.h"
}

	
/* transmit request task */
[[maybe_unused]] 
void RequestSendTask(void* argument)
{ 
    while (true)
    {
       vTaskDelay(pdMS_TO_TICKS(5000));
	   BSP_LED_Toggle(LED_BLUE);
	   DataTransmit::GetInstance().SendRquest(Packet::Level_request);
	   BSP_LED_Toggle(LED_BLUE);
    }
}

/* response handling task */
[[maybe_unused]] 
void ResponseHandlerTask(void* argument)
{ 
	DataTransmit::GetInstance().Init(&Radio,true); // Inicializace DataTransmitteru s nastavením MasterMode na true
	printf("Init device data  transmit\r");
    while (true)
    {
       /* wait for response */
		if(DataTransmit::GetInstance().DataAvailable){
			printf("Data received successfully, type: %d\n", DataTransmit::GetInstance().GetReceivedDataType());
			DataTransmit::GetInstance().DataAvailable = false; // Reset flag after processing
			if(DataTransmit::GetInstance().GetReceivedDataType()==Packet::Level_response){
				uint16_t level_value = parse_le<uint16_t>( DataTransmit::GetInstance().GetReceivedPayload(), 0);  
				printf("Received level value: %u cm\n", level_value);
				
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
	}
    }
