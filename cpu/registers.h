#pragma once

#include <common/common.h">

namespace gbemu
{
    enum IORegister {

        kP1 = 0xFF00, // joypad info
        kSB = 0xFF01, // serial transfer data
        kSC = 0xFF02, // serial transfer control
        kDIV = 0xFF04, // divider register
        kTIMA = 0xFF05, // timer counter
        kTMA = 0xFF06, // Timer modulo
        kTAC = 0xFF07, // Timer control
        kIF = 0xFF0F, // Interrupt flag

        kNR10 = 0xFF10,
        kNR11 = 0xFF11,
        kNR12 = 0xFF12,
        kNR13 = 0xFF13,
        kNR14 = 0xFF14,
        kNR21 = 0xFF16,
        kNR22 = 0xFF17,
        kNR23 = 0xFF18,
        kNR24 = 0xFF19,
        kNR30 = 0xFF1A,
        kNR31 = 0xFF1B,
        kNR32 = 0xFF1C,
        kNR33 = 0xFF1D,
        kNR34 = 0xFF1E,
        kNR41 = 0xFF20,
        kNR42 = 0xFF21,
        kNR43 = 0xFF22,
        kNR44 = 0xFF23,
        kNR50 = 0xFF24,
        kNR51 = 0xFF25,
        kNR52 = 0xFF26,
        kWavePatternRAMStart = 0xFF30,
        kWavePatternRAMEnd = 0xFF40,
        kSoundRegistersStart = kNR10,
        kSoundRegistersEnd = kWavePatternRAMEnd,

        kLCDC = 0xFF40,
        kSTAT = 0xFF41,

        kSCX = 0xFF43,
        kSCY = 0xFF42,

        kLY = 0xFF44,
        kLYC = 0xFF45,

        kDMA = 0xFF46,

        kBGP = 0xFF47,
        kOBP0 = 0xFF48,
        kOBP1 = 0xFF49,

        kWY = 0xFF4A,
        kWX = 0xFF4B,

        kIE = 0xFFFF,
    };
    enum WordRegister {
        kAF = 0,
        kBC = 1,
        kDE = 2,
        kHL = 3,
        kPC = 4,
        kSP = 5,
        kMemoryWordHL = 6,
        kMWPC = 7
    };
    enum ByteRegister {
        kA = 0,
        kB = 1,
        kC = 2,
        kD = 3,
        kE = 4,
        kF = 5,
        kH = 6,
        kL = 7,
        kMHL = 8,
        kMBC = 9,
        kMDE = 10,
        kMNN = 11,
        kMPC = 12
    };

    class Registers {
    public:
        Registers();

        unsigned char _A;

        bool _zero;
        bool _halfCarry;
        bool _carry;
        bool _substract;

        union {
            struct {
                union {
                    struct {
                        unsigned char c, b;
                    };
                    unsigned short word;
                } _BC;
                union {
                    struct {
                        unsigned char e, d;
                    };
                    unsigned short word;
                } _DE;
                union {
                    struct {
                        unsigned char l, h;
                    };
                    unsigned short word;
                } _HL;
                unsigned short _PC;
                unsigned short m_SP;
            };
        };

        JFX_INLINE unsigned short AF() const;
        JFX_INLINE void AF( unsigned short af );
    };

    unsigned short Registers::AF() const
    {
        return (unsigned short)(( _A << 8 ) | ( _zero << 7 ) | ( _substract << 6 ) | ( _halfCarry << 5 ) | ( _carry << 4 ));
    }

    void Registers::AF( unsigned short af )
    {
        _A = static_cast< unsigned char >( ( af & 0xFF00 ) >> 8 );
        _zero =      ( ( af & 0x0080 ) >> 7 ) != 0;
        _substract = ( ( af & 0x0040 ) >> 6 ) != 0;
        _halfCarry = ( ( af & 0x0020 ) >> 5 ) != 0;
        _carry =     ( ( af & 0x0010 ) >> 4 ) != 0;
    }
}
