/*
 * DataTransmit.cpp
 *
 *  Created on: 21. 4. 2026
 *      Author: radek
 */

#include  "DataTransmit.hpp"
#include  "RadioParams.hpp"
#include "timer.h"
#include <cassert>
#include <cstring>
#include <cstdio>

bool DataTransmit::MasterMode = false;
bool DataTransmit::RequestSent = false;
bool DataTransmit::DataAvailable = false;
bool DataTransmit::DataOverload = false;
const struct Radio_s* DataTransmit::RadioDriver = nullptr;
RadioEvents_t DataTransmit::RadioEvents = {};
TimerEvent_t DataTransmit::CadTimer = {};
Packet DataTransmit::packet = {};
bool DataTransmit::SlaveNotResponding = false;
Packet::PacketType DataTransmit::DataType = Packet::Type_undefined;
//DataTransmit::MasterMode = false;



extern "C"  [[maybe_unused]] 
void RadioCadTimeoutIrq( void *context ){
	if( DataTransmit::MasterMode == true){
		return; // pokud jsme v master modu, neprovádíme CAD
	}
	DataTransmit::RadioDriver->Standby( );
	DataTransmit::RadioDriver->SetChannel(CHANNEL);
	DataTransmit::RadioDriver->StartCad( );
	printf("Start CAD\n");
}

   
extern "C" void OnCadDone( bool channelActivityDetected ){
	if(channelActivityDetected){
		DataTransmit::RadioDriver->Rx(1200);
		TimerStop(&DataTransmit::CadTimer );
		printf(">>>>Start RX\n");
	}else{
		// Channel is clear, proceed with transmission
		TimerStart(&DataTransmit::CadTimer );
	}
}


void OnTxDone(void){
	if( DataTransmit::MasterMode == true){
		return; // pokud jsme v master modu, neprovádíme CAD
	}
	DataTransmit::RadioDriver->Standby( );
	DataTransmit::RadioDriver->SetChannel(CHANNEL);
	DataTransmit::RadioDriver->StartCad( );
	printf("Transmission done, restarting CAD\n");
};

/* Callback functions - Rx complete */
extern "C" void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t LoraSnr_FskCfo){
	printf("Received packet: size=%u, rssi=%d, snr=%d\n", size, rssi, LoraSnr_FskCfo);
	// kopírujeme payload do packetu pro další zpracování
	std::array<uint8_t, Packet::max_packet_size> buffer{};
	std::memcpy(buffer.data(), payload, size);
	if(DataTransmit::packet.ParsePacket(buffer)==true){
		if(DataTransmit::DataAvailable){
			DataTransmit::DataOverload = true;
		}
		else DataTransmit::DataOverload = false;
		DataTransmit::DataAvailable = true;
	}
	DataTransmit::RadioDriver->Standby( );
	DataTransmit::RadioDriver->SetChannel(CHANNEL);
	DataTransmit::RadioDriver->StartCad( );
}

extern "C" void OnTxTimeout(void){
	if( DataTransmit::MasterMode == true){
		return; // pokud jsme v master modu, neprovádíme CAD
	}
	DataTransmit::RadioDriver->Standby( );
	DataTransmit::RadioDriver->SetChannel(CHANNEL);
	DataTransmit::RadioDriver->StartCad( );
	printf("TX Timeout, restarting CAD\n");
}

extern "C" void OnRxTimeout(void){
	if( DataTransmit::MasterMode == true){
		if(DataTransmit::RequestSent){
			DataTransmit::RequestSent = false;
			DataTransmit::SlaveNotResponding = true;
		}
		return; // pokud jsme v master modu, neprovádíme CAD
	}
	DataTransmit::RadioDriver->Standby( );
	DataTransmit::RadioDriver->SetChannel(CHANNEL);
	DataTransmit::RadioDriver->StartCad( );
	printf("RX Timeout, restarting CAD\n");
}

extern "C" void OnRxError(void){
	if( DataTransmit::MasterMode == true){
		if(DataTransmit::RequestSent){
			DataTransmit::RequestSent = false;
			DataTransmit::SlaveNotResponding = true;
		}
		return; // pokud jsme v master modu, neprovádíme CAD
	}
	DataTransmit::RadioDriver->Standby( );
	DataTransmit::RadioDriver->SetChannel(CHANNEL);
	DataTransmit::RadioDriver->StartCad( );
	printf("RX Error, restarting CAD\n");
}

/* init  radio */
void DataTransmit::Init(const struct Radio_s *Radio_,bool MasterMode_){
	assert(Radio_ != nullptr); 
	RadioDriver = Radio_;
	MasterMode = MasterMode_;
	
	/* events  setup */
	RadioEvents.TxDone = OnTxDone;
	RadioEvents.RxDone = OnRxDone;
	RadioEvents.TxTimeout = OnTxTimeout;
	RadioEvents.RxTimeout = OnRxTimeout;
	RadioEvents.RxError = OnRxError;
	RadioEvents.CadDone = OnCadDone;
	RadioDriver->Init(&RadioEvents);

	RadioDriver->SetModem(MODEM_LORA);
	RadioDriver->SetPublicNetwork(true);
	RadioDriver->RxBoosted(true);	
	/* radio setup */
	RadioDriver->SetTxConfig(MODEM_LORA,TX_POWER,0,BANDWIDTH,SPREED_FACTOR,CODE_RATE,PREAMBLE_LEN,FIX_LEN,CRC_ON,
	  									FREQ_HOP_ON,HOP_PERIODE,SYMBOL_INVERTED,10000);
	RadioDriver->SetChannel(CHANNEL);
	RadioDriver->SetRxConfig(MODEM_LORA, BANDWIDTH,SPREED_FACTOR, CODE_RATE,0,PREAMBLE_LEN*2,SYMB_TIMEOUT,FIX_LEN,PAYLOAD_LEN,
	  										CRC_ON, FREQ_HOP_ON, HOP_PERIODE,SYMBOL_INVERTED,false);
	RadioDriver->Standby( );
 	
	/* CAD sampler timer setup */
	if(MasterMode == false){
		TimerInit( &CadTimer,RadioCadTimeoutIrq );
		TimerSetValue( &CadTimer,CAD_sample );
		TimerStart( &CadTimer );	
	}
	DataAvailable = false;		
	DataOverload = false;		
	printf("DataTransmit initialized\n");
}

/* odeslat požadavek na data */
bool DataTransmit::SendRquest(Packet::PacketType Type){
	
	/* create packet */
	if(!packet.CreatePacket(Type)){
		printf("Packet creation failed!\n");
		return false; // Packet creation failed
	}
	/* send packet */
	RequestSent = true;
	SlaveNotResponding = false;
	RadioDriver->Standby( );
	RadioDriver->SetChannel(CHANNEL);
	RadioDriver->Send(packet.Packet_output.data(), packet.Packet_output.size());
	while(RadioDriver->GetStatus( )==RF_TX_RUNNING){
		// čekáme na dokončení vysílání
	}
	RadioDriver->Rx(ResponseTimeout); // Po odeslání požadavku přepneme rádio do přijímacího režimu
	
	return true;
}

