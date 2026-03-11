#include "LFSR.h"

LFSR::LFSR(const std::array<uint8_t, REG_SIZE>& initialState) : reg(initialState) {}

uint8_t LFSR::nextBit()
{
    /* Remember the last bit */
    uint8_t output = reg[REG_SIZE - 1];

    /* Calculate the new bit */
    uint8_t feedback = reg[36] ^ reg[11] ^ reg[9] ^ reg[1];

    /* Shift the register */
    for (int i = REG_SIZE - 1; i > 0; i--)
        reg[i] = reg[i - 1];

    reg[0] = feedback;

    return output;
}

uint8_t LFSR::nextByte()
{
    uint8_t byte = 0;
    for (int i = 7; i >= 0; i--)
        byte |= (nextBit() << i);
    return byte;
}