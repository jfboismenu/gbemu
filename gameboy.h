#pragma once

#include "memory.h"
#include "cartridgeInfo.h"
#include "debugging.h"
#include "cpu.h"
#include "videoDisplay.h"
#include "bootRom.h"
#include "papu.h"

namespace gbemu {

    class Gameboy
    {
    public:
        Gameboy(const char* const bootRom);
        ~Gameboy();

        Memory& getMemory();
        CPU& getCPU();
        Cartridge& getCartridge();
        VideoDisplay& getVideo();
        DebugStringHandlerRegistry& getDebugRegistry();
        int doCycle();
        void registerSoundReadyCb(
            void (*callback)( const short* samples, int nbSamples ),
            void* clientData
        );
    private:

        void emulateTimers( int nbCycles );
        void handleInterrupts();

        static const int kCPUSpeed = 4194304;
        static const int kDividerFrequency = 16384;
        static const int kClockPerDividerCycle = kCPUSpeed / kDividerFrequency;

        int _cyclesToIncDivider;
        int _cyclesToIncTimerCounter;

        BootRom _bootRom;
        PAPU   _papu;
        Memory _memory;
        
        Cartridge _cartridge;
        DebugStringHandlerRegistry _debugReg;
        CPU _cpu;
        VideoDisplay _video;
        int _clock;
    };
}
