#pragma once

#include <memory/memory.h>
#include <memory/cartridgeInfo.h>
#include <memory/bootRom.h>
#include <cpu/cpu.h>
#include <video/videoDisplay.h>
#include <audio/papu.h>
#include <cpu/timers.h>
#include <base/clock.h>

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
        const CPUClock& getClock() const;
        int doCycle();

    private:

        void handleInterrupts();

        CPUClock _clock;
        BootRom _bootRom;
        PAPU   _papu;
        Memory _memory;

        Cartridge _cartridge;
        CPU _cpu;
        Timers _timers;

        VideoDisplay _video;
    };
}
