// gbemu.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <iostream>
#include <cassert>
#include <iomanip>
#include <sstream>
#include <memory>
#include <fstream>

#include <set>

#include <common/common.h>
#include <memory/cartridgeInfo.h>
#include <memory/memory.h>
#include <cpu/cpu.h>
#include <video/videoDisplay.h>
#include <gameboy.h>

namespace gbemu {

    std::unique_ptr< Gameboy > initGlobalEmulatorParams(
        const char* const filename,
        const char* const bootRom
    )
    {
        std::unique_ptr< Gameboy > gbInstance( new Gameboy( bootRom ) );

        gbInstance->getCartridge().Load( filename );
        gbInstance->getMemory().loadCartridge( gbInstance->getCartridge() );

        return gbInstance;
    }

    bool emulateSomeCycles( Gameboy& gbInstance, int nbCyclesToRun )
    {
        bool frameReady = false;
        int nbCyclesTotal = 0;

        while( !frameReady || nbCyclesTotal < nbCyclesToRun ) {
            // Take a snapshot of the registers for debugging purpose
            // const Registers registers( gbInstance.getCPU().getRegisters() );
            //debugCPU( registers );
            const int nbCycles = gbInstance.doCycle();
            nbCyclesTotal += nbCycles;
            frameReady = gbInstance.getVideo().isFrameReady();
            if ( nbCycles < 0 ) {
                break;
            }
        }
        // FIXME: The emulator was taking a huge performance hit when saving
        // ram to disk. Needs a better approach than the current one.
        // Once every frame?
        // gbInstance.getCartridge().saveRAM();
        return frameReady;
    }

}

