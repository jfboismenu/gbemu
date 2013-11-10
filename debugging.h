#pragma once

#include "opcode.h"
#include <ostream>
#include <string>

namespace gbemu
{
    typedef void (*OpcodeStringConverter)( std::ostream& os, const unsigned char* bytes );

    class DebugStringHandlerRegistry
    {
    public:
        DebugStringHandlerRegistry();
        void registerHandler(
            Opcode opcode,
            OpcodeStringConverter handler
        );
        OpcodeStringConverter getHandler( Opcode opcode ) const;
    private:
        OpcodeStringConverter _handlers[256];
        OpcodeStringConverter _cbHandlers[256];
    };

    void initializeDebugStringHandlers( DebugStringHandlerRegistry& reg );
}
