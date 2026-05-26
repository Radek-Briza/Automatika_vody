
#include "Packet.hpp"
#include <span>
#include <tuple>
#include <algorithm>
#include <ranges>
#include <cstring>
#include <cstdio>
#include "crc.h"
#include "stm32wlxx_hal_crc.h"
#include "stm32wlxx_hal_crc_ex.h"

Packet::Packet() = default;
Packet::~Packet() = default;

// Helper that returns a byte view over a POD value.
static auto GetAsByteArray(auto const& data) { return std::span{reinterpret_cast<const uint8_t*>(&data), sizeof(data)}; }

uint16_t calculate_crc(std::span<uint8_t> data,uint16_t initial_value = 0xFFFFu) {
    if (data.empty()) {
        return 0xFFFFu;
    }

    extern CRC_HandleTypeDef hcrc;
    CRC_HandleTypeDef modbus_crc = hcrc;

    modbus_crc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_DISABLE;
    modbus_crc.Init.GeneratingPolynomial = 0x8005U;            // CRC-16/MODBUS polynomial
    modbus_crc.Init.CRCLength = CRC_POLYLENGTH_16B;
    modbus_crc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_DISABLE;
    modbus_crc.Init.InitValue = initial_value;                      // MODBUS initial value
    modbus_crc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_BYTE;
    modbus_crc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_ENABLE;
    modbus_crc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;

    if (HAL_CRC_Init(&modbus_crc) != HAL_OK) {
        return 0xFFFFu;
    }
    uint16_t crc = HAL_CRC_Calculate(&modbus_crc,
                                     reinterpret_cast<uint32_t*>(data.data()),
                                     static_cast<uint32_t>(data.size()));
    return crc & 0xFFFFu;
}

/* Input/output buffer spans */
auto Packet::SegmentPacketToSpans(std::array<uint8_t,Packet::max_packet_size>& buffer){
    std::span ProtocolID = std::span{buffer}.first(Packet::packet_id_size); // Protocol ID is 2 bytes
    std::span Packet_ID = std::span{buffer}.subspan(Packet::packet_id_size,  Packet::packet_id_size); // Packet ID is 2 bytes
    std::span data_length = std::span{buffer}.subspan((Packet::packet_id_size*2), Packet::data_length_size); // Data Length is 2 bytes
    std::span CRC_data = std::span{buffer}.subspan((Packet::packet_id_size*2)+Packet::data_length_size, Packet::crc_size); // CRC is 2 bytes (CRC16)
    std::span Payload = std::span{buffer}.subspan(Packet::header_size); // The rest is payload
    return std::make_tuple(ProtocolID,Packet_ID, data_length, CRC_data, Payload);         
}

/* create packet without payload*/
bool Packet::CreatePacket(PacketType Type ){
   std::vector<uint8_t> empty_payload={}; // Empty payload
   Packet_output.clear(); // Clear output buffer before creating new packet
   auto [ProtocolID, Packet_ID, data_length, CRC_data, Payload] = Packet::SegmentPacketToSpans(PacketData);
   Packet_output.clear(); // Clear output buffer before creating new packet
   uint16_t Protocol__ID =Packet::Protocol_ID; // Example Protocol ID
   std::ranges::copy(GetAsByteArray(Protocol__ID), ProtocolID.begin()); // Copy Protocol ID to packet
   std::ranges::copy(GetAsByteArray(Type), Packet_ID.begin()); // Copy Packet ID to packet 
   std::ranges::copy(empty_payload, Payload.begin()); // Copy Payload to packet  
   uint16_t payload_size = 0;
   std::ranges::copy(GetAsByteArray(payload_size), data_length.begin()); // Copy Data Length to packet
   auto Header = std::span{PacketData}.first(Packet::header_size-Packet::crc_size); // - crc area
   uint16_t crc = calculate_crc(Header); // Calculate CRC over header 
   printf("Calculated CRC: 0x%04X\n", crc); // Print calculated CRC for debugging
   std::ranges::copy(GetAsByteArray(crc), CRC_data.begin());
   std::ranges::copy_n(PacketData.data(), Packet::header_size, std::back_inserter(Packet_output)); // Copy header to output vector
   printf("packet size: %d\n", static_cast<unsigned int>(Packet_output.size()));
   return true; // Packet created successfully
}


/* create packet with given type and payload */
 bool Packet::CreatePacket(PacketType Type, std::vector<uint8_t> &Payload_input){
    if(Payload_input.size() > Packet::max_payload_size || Payload_input.empty())  {
        printf("Payload size error!\n");     
        return false; // Payload too large to fit in packet
    }
    Packet_output.clear(); // Clear output buffer before creating new packet
   auto [ProtocolID, Packet_ID, data_length, CRC_data, Payload] = Packet::SegmentPacketToSpans(PacketData);
   std::ranges::copy(GetAsByteArray(Protocol_ID), ProtocolID.begin()); // Copy Protocol ID to packet
   std::ranges::copy(GetAsByteArray(Type), Packet_ID.begin()); // Copy Packet ID to packet 
   std::ranges::copy(Payload_input, Payload.begin()); // Copy Payload to packet  
   uint16_t payload_size = static_cast<uint16_t>(Payload_input.size());
   std::ranges::copy(GetAsByteArray(payload_size), data_length.begin()); // Copy Data Length to packet
   auto Header = std::span{PacketData}.first(Packet::header_size-Packet::crc_size); // - crc area
   uint16_t crc = calculate_crc(Header); // Calculate CRC over header and payload
   crc = calculate_crc(Payload, crc); // Calculate CRC over header and payload
   printf("Calculated CRC: 0x%04X\n", crc); // Print calculated CRC for debugging
   std::ranges::copy(GetAsByteArray(crc), CRC_data.begin());
   std::ranges::copy_n(PacketData.data(), Packet::header_size+Payload_input.size(), std::back_inserter(Packet_output)); // Copy header to output vector
   return true; // Packet created successfully
 }

 /* parse packet */
 bool Packet::ParsePacket(std::array<uint8_t, Packet::max_packet_size> &Packet_inp ){ 
    // ZISKANI SPANU PRO KAZDOU CAST PACKETU
    auto [ProtocolID, Packet_ID, data_length, CRC_data, Payload] = Packet::SegmentPacketToSpans(const_cast<std::array<uint8_t, Packet::max_packet_size>&>(Packet_inp));
    
    // KONVERZE SPANU NA PRIMITIVNI TYPY
    auto Protocol_ID_value = *reinterpret_cast<const uint16_t*>(ProtocolID.data()); // Convert Protocol ID span to uint16_t 
    if(Protocol_ID_value != Protocol_ID){ // Assuming 0xABCD is the expected Protocol ID
        printf("Invalid Protocol ID: 0x%04X\n", Protocol_ID_value);
        return false; // Invalid Protocol ID
    }
    auto m_paket_ID =    *reinterpret_cast<const uint16_t*>(Packet_ID.data()); // Convert data length span to uint16_t
    auto m_data_length = *reinterpret_cast<const uint16_t*>(data_length.data()); // Convert data length span to uint16_t
    auto m_crc_data =    *reinterpret_cast<const uint16_t*>(CRC_data.data()); // Convert   CRC span to uint16_t    
    std::array<uint8_t, Packet::max_packet_size> m_payload{};
    std::ranges::copy(Payload, m_payload.data());
    
    printf("Packet type: %u\n", m_paket_ID);
    printf("Data Length: %u\n", m_data_length);       
    
    // verify CRC
    auto Header = std::span{Packet_inp}.first(Packet::header_size-Packet::crc_size); // - crc area
    uint16_t calculated_crc = calculate_crc(Header); // Calculate CRC over header and payload
    calculated_crc = calculate_crc(Payload.first(m_data_length), calculated_crc); // Calculate CRC over header and payload
        if (calculated_crc != m_crc_data) {
                printf("CRC check failed! Expected: 0x%04X, Calculated: 0x%04X\n", static_cast<unsigned int >(m_crc_data), static_cast<unsigned int>(calculated_crc));
                return false; // CRC check failed
            }
            printf("CRC check passed!\n");
            Payload_output.clear(); // Clear previous payload before copying new data
            std::ranges::copy(Payload.first(m_data_length), std::back_inserter(Payload_output)); // Copy payload to output vector

            printf("Received payload: ");
            for( const auto& byte : Payload_output){
                printf("%02X ", byte);
            }
            printf("\n");
            Type_out = static_cast<PacketType>(m_paket_ID); // A
    return true; // Packet parsed successfully
}
