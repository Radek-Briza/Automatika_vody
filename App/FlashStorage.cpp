#include "main.h"
#include "FlashStorage.hpp"
#include <vector>
#include <cstring>


extern CRC_HandleTypeDef hcrc;

uint32_t FlashParameterStorage::CalculateCrc32(
    const uint8_t* data,
    size_t size){

    return HAL_CRC_Calculate(
        &hcrc,
        reinterpret_cast<uint32_t*>(
            const_cast<uint8_t*>(data)),
        (size + 3u) / 4u);
}

/* erase flash page */
bool FlashParameterStorage::EraseFlashPage()
{
    HAL_FLASH_Unlock();

    FLASH_EraseInitTypeDef eraseInit{};
    eraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
    eraseInit.Page = (flashAddress_ - FLASH_BASE) / FLASH_PAGE_SIZE;
    eraseInit.NbPages = 1;

    uint32_t pageError;

    bool success =
        (HAL_FLASHEx_Erase(&eraseInit, &pageError) == HAL_OK);

    HAL_FLASH_Lock();

    return success;
}

/* init load record from flash */
bool FlashParameterStorage::LoadFromFlash(){
    cache_.clear();

    auto* hdr = reinterpret_cast<const FlashHeader*>(flashAddress_); /* pointer to flash header */

    if(hdr->magic != MAGIC) {
        recordCount_ = 0;
        return false;
    }

    if(hdr->version != VERSION){
        recordCount_ = 0;
        return false;
    }

    auto* records = reinterpret_cast<const FlashRecord*>(flashAddress_ + sizeof(FlashHeader)); /* pointer to flash records */

    uint32_t crc =
        CalculateCrc32(
            reinterpret_cast<const uint8_t*>(records),
            hdr->recordCount * sizeof(FlashRecord));

    if(crc != hdr->crc32){
        recordCount_ = 0;
        return false;
    }

    for(uint16_t i = 0; i < hdr->recordCount; ++i) {
        cache_[records[i].name] = records[i].value;
    }

    recordCount_ = hdr->recordCount;

    return true;
}

bool FlashParameterStorage::SaveToFlash(){
    std::vector<uint8_t> image;

    image.resize(
        sizeof(FlashHeader) +
        cache_.size() * sizeof(FlashRecord));

    auto* hdr =reinterpret_cast<FlashHeader*>(image.data());

    hdr->magic       = MAGIC;
    hdr->version     = VERSION;
    hdr->recordCount = cache_.size();
    hdr->crc32       = 0;

    auto* records = reinterpret_cast<FlashRecord*>(image.data() +sizeof(FlashHeader));

    uint32_t index = 0;

    for(const auto& [name,value] : cache_){
        std::memset(&records[index], 0,sizeof(FlashRecord));

        std::strncpy(records[index].name,name.c_str(),sizeof(records[index].name)-1);

        records[index].value = value;

        ++index;
    }

    hdr->crc32 =
        CalculateCrc32(
            reinterpret_cast<uint8_t*>(records),
            cache_.size() *
            sizeof(FlashRecord));

    while(image.size() % 8) {
        image.push_back(0xFF);
    }

    if(!EraseFlashPage()) {
        return false;
    }

    HAL_FLASH_Unlock();

    for(size_t i=0; i<image.size();  i+=8) {
        uint64_t dw;

        std::memcpy(
            &dw,
            image.data() + i,
            sizeof(dw));

        if(HAL_FLASH_Program(
            FLASH_TYPEPROGRAM_DOUBLEWORD,
            flashAddress_ + i,
            dw) != HAL_OK){
            HAL_FLASH_Lock();
            return false;
        }
    }

    HAL_FLASH_Lock();

    return true;
}

/* constructor */
FlashParameterStorage::FlashParameterStorage(
    uint32_t flashAddress)
    : flashAddress_(flashAddress){
    LoadFromFlash();
}

/* read record */
bool FlashParameterStorage::ReadRecord(
    std::map<std::string,int32_t>& records){
    //bool found = false;
    auto expect_records_count_ = records.size();

    for(auto& [name,value] : records)
    {
        auto it = cache_.find(name);

        if(it != cache_.end()){
            value = it->second;
           expect_records_count_--;
        }
    }

    return expect_records_count_ == 0;
}

bool FlashParameterStorage::WriteRecord(
    const std::map<std::string,int32_t>& records){
    bool modified = false;

    for(const auto& [name,value] : records){
        if(name.length() > 31){
            continue;
        }

        auto it = cache_.find(name);

        if(it == cache_.end()){
            cache_[name] = value;
            modified = true;
        }
        else if(it->second != value){
            it->second = value;
            modified = true;
        }
    }

    if(!modified) {
        return true;
    }

    recordCount_ =
        static_cast<uint16_t>(
            cache_.size());

    return SaveToFlash();
}

/* erase record */
bool FlashParameterStorage::EraseRecord(
    const std::map<std::string,int32_t>& records)
{
    bool removed = false;

    for(const auto& [name,value] : records)
    {
        auto it = cache_.find(name);

        if(it != cache_.end())
        {
            cache_.erase(it);
            removed = true;
        }
    }

    if(!removed)
{
        return false;
    }

    recordCount_ =
        static_cast<uint16_t>(
            cache_.size());

    return SaveToFlash();
}

/* format flash */
bool FlashParameterStorage::Format()
{
    cache_.clear();
    recordCount_ = 0;

    return EraseFlashPage();
}