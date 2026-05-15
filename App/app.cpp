/*
 * app.cpp
 *
 *  Created on: 11. 4. 2026
 *      Author: radek
 */
#include "main.h"
#include  "App.hpp"
#include "stdlib.h"
#include "radio.h"	
#include "stdio.h"
#include  "DataTransmit.hpp"

 
void App::init()
{
	DataTransmit::GetInstance().Init(&Radio,true); // Inicializace DataTransmitteru s nastavením MasterMode na true
	printf("Init device\r");
}
	


void App::loop()
{
	for(;;){
		HAL_Delay(5000);
		BSP_LED_Toggle(LED_BLUE);
		DataTransmit::GetInstance().SendRquest(Packet::Level_request);
		BSP_LED_Toggle(LED_BLUE);
		/* wait for response */
		
		if(DataTransmit::GetInstance().DataAvailable){
			printf("Data received successfully, type: %d\n", DataTransmit::GetInstance().GetReceivedDataType());
			DataTransmit::GetInstance().DataAvailable = false; // Reset flag after processing
			if(DataTransmit::GetInstance().GetReceivedDataType()==Packet::Level_response){
				uint16_t level_value = parse_le<uint16_t>( DataTransmit::GetInstance().GetReceivedPayload(), 0);  
				printf("Received level value: %u cm\n", level_value);
				
			}
			BSP_LED_Toggle(LED_GREEN);
			HAL_Delay(100);
			BSP_LED_Toggle(LED_GREEN);

		}
		else if(DataTransmit::GetInstance().DataOverload){
			printf("Data overload! New data received before processing previous data.\n");
			DataTransmit::GetInstance().DataOverload = false; // Reset flag after handling overload
			BSP_LED_Toggle(LED_RED);
			HAL_Delay(100);
			BSP_LED_Toggle(LED_RED);
		}
		else if(DataTransmit::GetInstance().SlaveNotResponding){
			printf("Slave not responding! No response received within timeout period.\n");
			DataTransmit::GetInstance().SlaveNotResponding = false; // Reset flag after handling no response
			BSP_LED_Toggle(LED_RED);
			HAL_Delay(100);
			BSP_LED_Toggle(LED_RED);
		}
	}

}
