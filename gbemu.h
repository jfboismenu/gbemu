//
//  gbemu.h
//  gbemu
//
//  Created by JF Boismenu on 2012-10-09.
//
//

#ifndef gbemu_gbemu_h
#define gbemu_gbemu_h

#include <gameboy.h>

namespace gbemu {

    enum OutputEvent
    {
        NoEvent = 0,
        VideoFrameReady = 1
    };

    std::unique_ptr< Gameboy > initGlobalEmulatorParams(
        const char* const filename,
        const char* const bootRomPath
    );

    bool emulateSomeCycles( Gameboy& gb, int nbCyclesToRun );

}


#endif
