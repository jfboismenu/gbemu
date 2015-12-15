#pragma once

#include "memory.h"
#include "cartridgeInfo.h"
#include "debugging.h"
#include "cpu.h"
#include "videoDisplay.h"
#include "bootRom.h"
#include "papu.h"
#include "timers.h"
#include "clock.h"

namespace gbemu {

    class Gameboy
    {
    public:
        Gameboy(const char* const bootRom);
        ~Gameboy();

        Memory& getMemory();
        CPU& getCPU();
        PAPU& getPAPU();
        Cartridge& getCartridge();
        VideoDisplay& getVideo();
        DebugStringHandlerRegistry& getDebugRegistry();
        const Clock& getClock() const;
        int doCycle();

    private:

        void handleInterrupts();

        Clock _clock;
        BootRom _bootRom;
        PAPU   _papu;
        Memory _memory;
        
        Cartridge _cartridge;
        DebugStringHandlerRegistry _debugReg;
        CPU _cpu;
        Timers _timers;
        
        VideoDisplay _video;
    };
}
