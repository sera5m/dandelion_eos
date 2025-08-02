#ifndef HELPERFUNCTIONS_H
#define HELPERFUNCTIONS_H

#include <cstdint>
#include "types.h"
#include <cstddef>
//bits
#pragma once

void split_u32_to_24(uint32_t input, uint8_t *last8, uint8_t *color);

inline int16_t wrap_value(int16_t value, int16_t min, int16_t max) {
    int16_t range = max - min + 1;
    if (range <= 0) return min;  // Defensive
    while (value < min) value += range;
    while (value > max) value -= range;
    return value;
}

//templates
template<typename T>
inline T CLAMP(const T& val, const T& min, const T& max) {
    return (val < min) ? min : (val > max) ? max : val;
}

#endif//end HELPERFUNCTIONS_H
