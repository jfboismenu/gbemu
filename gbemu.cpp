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

#include "common.h"
#include "cartridgeInfo.h"
#include "debugging.h"
#include "memory.h"
#include "cpu.h"
#include "videoDisplay.h"
#include "gameboy.h"

namespace gbemu {

/*
    template< typename T >
    bool printRegister( std::ostream& os, const char* regName, const T oldValue, const T newValue )
    {
        if ( oldValue != newValue ) {
            os << " " << regName << "=" << std::hex << (int)newValue;
            return true;
        }
        return false;
    }

    void printRegisters( std::ostream& os, const Registers& oldReg, const Registers& newReg )
    {
        printRegister( os, "AF", oldReg.AF(), newReg.AF() );
        printRegister( os, "BC", oldReg._BC.word, newReg._BC.word );
        printRegister( os, "DE", oldReg._DE.word, newReg._DE.word );
        printRegister( os, "HL", oldReg._HL.word, newReg._HL.word );
        printRegister( os, "SP", oldReg.m_SP, newReg.m_SP );
//            printRegister( os, "PC", oldReg._PC, newReg._PC );
        printRegister( os, "A", oldReg._A, newReg._A );
        printRegister( os, "B", oldReg._BC.b, newReg._BC.b );
        printRegister( os, "C", oldReg._BC.c, newReg._BC.c );
        printRegister( os, "D", oldReg._DE.d, newReg._DE.d );
        printRegister( os, "E", oldReg._DE.e, newReg._DE.e );
        printRegister( os, "H", oldReg._HL.h, newReg._HL.h );
        printRegister( os, "L", oldReg._HL.l, newReg._HL.l );
        printRegister( os, "Z", oldReg._zero, newReg._zero );
        printRegister( os, "N", oldReg._substract, newReg._substract );
        printRegister( os, "C", oldReg._carry, newReg._carry );
        printRegister( os, "H", oldReg._halfCarry, newReg._halfCarry );
    }


    void debug( std::ostream& os, CPU& cpu, Memory& memory, DebugStringHandlerRegistry& debugReg ) 
    {
        // Print debugging information for that opcode if available
        const Opcode opcode( cpu.previewOpcode() );
        Registers registerCopy( cpu.getRegisters() );

        OpcodeStringConverter handler( debugReg.getHandler( opcode ) );
        if ( !handler ) {
            os << "No debug information for " << std::hex << (unsigned int)opcode << " at " << std::hex << registerCopy._PC;
        }
        else {
            os << std::setw( 6 ) << std::hex << registerCopy._PC << " ";
            (*handler)( os, memory.getBytes( registerCopy._PC ) );
        }
        os << " |";
    }
*/
//    std::set< unsigned short > addresses;

//    std::ofstream fileOut( "c:/Users/JF/instructionDump.txt" );
//    std::ostream& os( fileOut );

    std::unique_ptr< Gameboy > initGlobalEmulatorParams( 
        const char* const filename,
        const char* const bootRom
    )
    {
        std::unique_ptr< Gameboy > gbInstance( new Gameboy( bootRom ) );
        
        gbInstance->getCartridge().Load( filename );
        gbInstance->getMemory().loadCartridge( gbInstance->getCartridge() );
        initializeDebugStringHandlers( gbInstance->getDebugRegistry() );
        
        return gbInstance;
    }

 //   void debugCPU( const Registers& /*registers*/ )
 //   {
 //       // if we are booting rom, don't bother logging instructions
 //       if ( gbInstance.getCPU().inBootRom() ) {
 //           return;
 //       }
 //       /*if ( !addresses.insert( registers._PC ).second ) {
 //           return;
 //       }*/
 //       // Print debug information
 //   //debug( os, _cpu, _memory, _debugReg );
 //       //printRegisters( os, registers, _cpu.getRegisters() );
 //      // os << std::endl;
 //   }
    
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
        gbInstance.getCartridge().saveRAM();
        return frameReady;
    }
    
}

