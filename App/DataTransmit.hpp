/*
 * DataTransmit.hpp
 *
 *  Created on: 21. 4. 2026
 *      Author: radek
 */

#ifndef DATATRANSMIT_HPP_
#define DATATRANSMIT_HPP_

#include "radio.h"
#include "timer.h"
#include "Packet.hpp"

#ifdef __cplusplus
extern "C" {
#endif

void RadioCadTimeoutIrq(void *context);
void OnCadDone(bool channelActivityDetected);
void OnTxDone(void);
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t LoraSnr_FskCfo);
void OnRxTimeout(void);
void OnRxError(void);
void OnTxTimeout(void);
#ifdef __cplusplus
}
#endif

class DataTransmit {	
public:
	static DataTransmit& GetInstance() {
		static DataTransmit instance;
		return instance;
	}
	void Init(const struct Radio_s *Radio_, bool MasterMode_ = false);
	bool SendRquest(Packet::PacketType Type);
	 Packet::PacketType GetReceivedDataType() const { return DataType; }
	static bool DataAvailable;
	static bool DataOverload; 
	static bool RequestSent;
	static bool MasterMode;
	static bool SlaveNotResponding;
	static Packet::PacketType ReceivedDataType;
	

private:
	DataTransmit() = default;
	~DataTransmit() = default;
	DataTransmit(const DataTransmit&) = delete;
	DataTransmit& operator=(const DataTransmit&) = delete;
	DataTransmit(DataTransmit&&) = delete;
	DataTransmit& operator=(DataTransmit&&) = delete;

	const uint32_t MaxPayloadSize = Packet::max_packet_size;
	static const struct Radio_s *RadioDriver;
	static const uint32_t CAD_sample = 200U; /* ms*/
	static  const uint32_t ResponseTimeout = 5000U; /* ms*/
	static RadioEvents_t RadioEvents;
	static TimerEvent_t CadTimer;
	static Packet packet;
	//static std::vector<uint8_t> Data; // Buffer pro příjem dat z rádia
    static Packet::PacketType DataType; // Proměnná pro uložení typu dat z příchozího packetu

	friend void RadioCadTimeoutIrq(void *context);
	friend void OnCadDone(bool channelActivityDetected);
	friend void OnTxTimeout(void);
	friend void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t LoraSnr_FskCfo);
	friend void OnRxTimeout(void);
	friend void OnRxError(void);
	friend void OnTxDone(void);
};

#endif /* DATATRANSMIT_HPP_ */
