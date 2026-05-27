
#ifndef PACKET_HPP_
#define PACKET_HPP_

#include <array>
#include <cstdint>
#include <cstring>
#include <vector>


class Packet
{   
    public:
        Packet()=default;
        ~Packet()=default;

    enum PacketType{
            Type_undefined = 0x00,
            Level_request = 0x01,
            Level_response = 0x02,
            Battery_request = 0x03,
            Battery_response = 0x04,
        };
    
    static const uint16_t Protocol_ID = 0x1221; // E
    static const int max_packet_size = 50; // Example size for header metadata
    static const int packet_id_size = 2; // Example size for Packet ID
    static const int data_length_size = 2; // Example size for Data Length
    static const int crc_size = 2; // CRC16 size for MODBUS
    static const int header_size = (packet_id_size*2)+data_length_size+crc_size ; // Header metadata size   
    static const int  max_payload_size = max_packet_size - header_size; // Remaining 
    std::vector<uint8_t> Packet_output = {}; // Output packet buffer
    std::vector<uint8_t> Payload_output = {}; // Buffer for received payload from radio
    Packet::PacketType Type_out = Packet::Type_undefined; // Variable for received packet type from radio

    bool CreatePacket(Packet::PacketType Type, std::vector<uint8_t> &Payload_input);
    bool CreatePacket(Packet::PacketType Type);
    bool ParsePacket(std::array<uint8_t, Packet::max_packet_size> & Packet_inp );
        
    private:
    auto SegmentPacketToSpans(std::array<uint8_t,Packet::max_packet_size>& buffer) ;    
    std::array<uint8_t, max_packet_size> PacketData; // Output packet buffer
    uint16_t ID; // Packet ID         
};  

#endif