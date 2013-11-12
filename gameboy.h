#pragma once

#include "memory.h"
#include "cartridgeInfo.h"
#include "debugging.h"
#include "cpu.h"
#include "videoDisplay.h"
#include "bootRom.h"
#include "papu.h"
#include "timers.h"

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

        void handleInterrupts();

        BootRom _bootRom;
        PAPU   _papu;
        Memory _memory;
        
        Cartridge _cartridge;
        DebugStringHandlerRegistry _debugReg;
        CPU _cpu;
        VideoDisplay _video;
        Timers _timers;
        int _clock;
    };
}
