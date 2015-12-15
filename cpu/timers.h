#pragma once

namespace gbemu {

    class Memory;

    class Timers
    {
    public:
        Timers( Memory& memory );

        bool contains( unsigned short addr ) const;
        unsigned char readByte( unsigned short addr ) const;
        void writeByte( unsigned short addr, unsigned char value );
        void emulate( int nbCycles );

    private:

        static const int kCPUSpeed = 4194304;
        static const int kDividerFrequency = 16384;
        static const int kClockPerDividerCycle = kCPUSpeed / kDividerFrequency;

        int _cyclesToIncDivider;
        int _cyclesToIncTimerCounter;

        unsigned char _tma;
        unsigned char _tima;
        unsigned char _div;
        unsigned char _tac;
        Memory&       _memory;
    };
}