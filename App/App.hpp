/*
 * App.hpp
 *
 *  Created on: 19. 4. 2026
 *      Author: radek
 */
#pragma once 

#include "main.h" // IWYU pragma: keep.
#include <cstdint>
#include <vector>
#include <type_traits>
#include "stdlib.h"
#include "Common.hpp"


/* Payload parsing functions */
template<typename T>
inline uint32_t ParsePayload(const std::vector<uint8_t>& data, std::size_t index){
    static_assert(
        std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t> || std::is_same_v<T, uint32_t>,
        "Supported types: uint8_t, uint16_t, uint32_t"
    );

    constexpr std::size_t type_size = sizeof(T);
    const std::size_t offset = index * type_size;

    if (offset + type_size > data.size()){
       #if APP_DEBUG_PRINT
       printf("Error: Attempt to read beyond data bounds. Data size: %d, Requested offset: %d, Type size: %d\n",
       data.size(), offset, type_size);
       #endif
       return std::numeric_limits<uint32_t>::max(); // Return max value to indicate error, or consider throwing an exception
    }

    uint32_t result = 0;

    for (std::size_t i = 0; i < type_size; ++i){
        result |= static_cast<uint32_t>(data[offset + i]) << (8 * i);
    }

    return result;
}


extern "C" {
    void RequestSendTask(void* argument);
    void ResponseHandlerTask(void* argument);
    void InitApplication(void);
    void InitPumpSystem(void);
    void  InitStorage(void);
}

