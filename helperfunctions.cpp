#include "helperfunctions.h"
#include "types.h"
#include <cstdint>
//bitsmushing here
#include <cstddef>


void split_u32_to_24(uint32_t input, uint8_t *last8, uint8_t *color) {
    // Extract 24-bit color (lower 24 bits)
    color[0] = (input >> 16) & 0xFF; // Red component
    color[1] = (input >> 8) & 0xFF;  // Green component
    color[2] = input & 0xFF;         // Blue component

    // Extract last 8 bits (most significant byte)
    *last8 = (input >> 24) & 0xFF;   // Days bitmask
}



//far as i know the clamp template only goes in the h