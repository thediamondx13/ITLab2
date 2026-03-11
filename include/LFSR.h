#pragma once

#include <QByteArray>
#include <array>
#include <cassert>

/* Polynomial: x^37 + x^12 + x^10 + x^2 + 1 */
class LFSR
{
public:
    static constexpr int REG_SIZE = 37;

    explicit LFSR(const std::array<uint8_t, REG_SIZE>& initialState);

    /* Returns single output bit
     and advances the register */
    uint8_t nextBit();

    /* Returns one byte of keystream */
    uint8_t nextByte();

private:
    std::array<uint8_t, REG_SIZE> reg;
};
