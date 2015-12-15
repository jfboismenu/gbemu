#include <cpu/registers.h>

namespace gbemu {
    Registers::Registers()
    {
        _PC = 0x100;
         m_SP = 0xFFFE;
        _A = 0x01;
        _zero = true;
        _substract = false;
        _halfCarry = true;
        _carry = true;
        _BC.word = 0x0013;
        _DE.word = 0x00D8;
        _HL.word = 0x014D;
    }
}
