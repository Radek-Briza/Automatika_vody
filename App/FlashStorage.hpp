#pragma once

#include <map>
#include <string>


class FlashParameterStorage
{
public:

    explicit FlashParameterStorage(uint32_t flashAddress);

    bool WriteRecord(
        const std::map<std::string,int32_t>& records);

    bool ReadRecord(
        std::map<std::string,int32_t>& records);

    bool EraseRecord(
        const std::map<std::string,int32_t>& records);

    bool Format();

    uint16_t GetRecordCount() const{ return recordCount_; }

    private:
    static constexpr uint32_t MAGIC   = 0x5041524D; // PARM
    static constexpr uint16_t VERSION = 1;

    
    struct FlashHeader{
        uint32_t magic;
        uint16_t version;
        uint16_t recordCount;
        uint32_t crc32;
    };

    struct FlashRecord{
    char    name[32];
    int32_t value;
    };

    static_assert(sizeof(FlashHeader) == 12);
    static_assert(sizeof(FlashRecord) == 36);

    bool LoadFromFlash();
   
    bool SaveToFlash();
   
    bool EraseFlashPage();

    uint32_t CalculateCrc32(const uint8_t* data,size_t size);

    bool IsFlashValid() const;

    uint32_t flashAddress_;

    uint16_t recordCount_{0};

    std::map<std::string,int32_t> cache_;
};