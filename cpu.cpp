#include "cpu.h"
#include "cartridgeInfo.h"
#include "memory.h"
#include <iostream>

namespace gbemu {

    CPU::CPU(
        Memory& memory,
        Cartridge& cartridge
    ) : _memory( memory ),
        _cartridge( cartridge ),
        _interruptState( kDisabled ),
        _isHalted( false )
    {
        // initialize the cycles table
        memset(_opTime, 0x04, sizeof(_opTime));
        _opTime[0x01]=12;_opTime[0x02]= 8;_opTime[0x03]= 8;_opTime[0x06]= 8;_opTime[0x08]=20;_opTime[0x09]= 8;_opTime[0x0A]= 8;_opTime[0x0B]= 8;
        _opTime[0x0E]= 8;_opTime[0x11]=12;_opTime[0x12]= 8;_opTime[0x13]= 8;_opTime[0x16]= 8;_opTime[0x18]= 8;_opTime[0x19]= 8;_opTime[0x1A]= 8;
        _opTime[0x1B]= 8;_opTime[0x1E]= 8;_opTime[0x20]= 8;_opTime[0x21]=12;_opTime[0x22]= 8;_opTime[0x23]= 8;_opTime[0x26]= 8;_opTime[0x28]= 8;
        _opTime[0x29]= 8;_opTime[0x2A]= 8;_opTime[0x2B]= 8;_opTime[0x2E]= 8;_opTime[0x30]= 8;_opTime[0x31]=12;_opTime[0x32]= 8;_opTime[0x33]= 8;
        _opTime[0x34]=12;_opTime[0x35]=12;_opTime[0x36]=12;_opTime[0x38]= 8;_opTime[0x39]= 8;_opTime[0x3A]= 8;_opTime[0x3B]= 8;_opTime[0x3E]= 8;
        _opTime[0x46]= 8;_opTime[0x4E]= 8;_opTime[0x56]= 8;_opTime[0x5E]= 8;_opTime[0x66]= 8;_opTime[0x6E]= 8;_opTime[0x70]= 8;_opTime[0x71]= 8;
        _opTime[0x72]= 8;_opTime[0x73]= 8;_opTime[0x74]= 8;_opTime[0x75]= 8;_opTime[0x77]= 8;_opTime[0x7E]= 8;_opTime[0x86]= 8;_opTime[0x8E]= 8;
        _opTime[0x96]= 8;_opTime[0x9E]= 8;_opTime[0xA6]= 8;_opTime[0xAE]= 8;_opTime[0xB6]= 8;_opTime[0xBE]= 8;_opTime[0xC0]= 8;_opTime[0xC1]=12;
        _opTime[0xC2]=12;_opTime[0xC3]=12;_opTime[0xC4]=12;_opTime[0xC5]=16;_opTime[0xC6]= 8;_opTime[0xC7]=32;_opTime[0xC8]= 8;_opTime[0xC9]= 8;
        _opTime[0xCA]=12;_opTime[0xCC]=12;_opTime[0xCD]=12;_opTime[0xCE]= 8;_opTime[0xCF]=32;_opTime[0xD0]= 8;_opTime[0xD1]=12;_opTime[0xD2]=12;
        _opTime[0xD4]=12;_opTime[0xD5]=16;_opTime[0xD6]= 8;_opTime[0xD7]=32;_opTime[0xD8]= 8;_opTime[0xD9]= 8;_opTime[0xDA]=12;_opTime[0xDC]=12;
        _opTime[0xDF]=32;_opTime[0xE0]=12;_opTime[0xE1]=12;_opTime[0xE2]= 8;_opTime[0xE5]=16;_opTime[0xE6]= 8;_opTime[0xE7]=32;_opTime[0xE8]=16;
        _opTime[0xEA]=16;_opTime[0xEE]= 8;_opTime[0xEF]=32;_opTime[0xF0]=12;_opTime[0xF1]=12;_opTime[0xF2]= 8;_opTime[0xF5]=16;_opTime[0xF6]= 8;
        _opTime[0xF7]=32;_opTime[0xF8]=12;_opTime[0xF9]= 8;_opTime[0xFA]=16;_opTime[0xFE]= 8;_opTime[0xFF]=32;

        // Initialize cycles for the special opcode 0xcb
        memset(_opTimeCb, 0x08, sizeof(_opTimeCb));
        _opTimeCb[0x06]=16;_opTimeCb[0x0e]=16;_opTimeCb[0x16]=16;_opTimeCb[0x1e]=16;_opTimeCb[0x26]=16;_opTimeCb[0x2e]=16;_opTimeCb[0x36]=16;_opTimeCb[0x3e]=16;
        _opTimeCb[0x46]=16;_opTimeCb[0x4e]=16;_opTimeCb[0x56]=16;_opTimeCb[0x5e]=16;_opTimeCb[0x66]=16;_opTimeCb[0x6e]=16;_opTimeCb[0x76]=16;_opTimeCb[0x7e]=16;
        _opTimeCb[0x86]=16;_opTimeCb[0x8e]=16;_opTimeCb[0x96]=16;_opTimeCb[0x9e]=16;_opTimeCb[0xa6]=16;_opTimeCb[0xae]=16;_opTimeCb[0xb6]=16;_opTimeCb[0xbe]=16;
        _opTimeCb[0xc6]=16;_opTimeCb[0xce]=16;_opTimeCb[0xd6]=16;_opTimeCb[0xde]=16;_opTimeCb[0xe6]=16;_opTimeCb[0xee]=16;_opTimeCb[0xf6]=16;_opTimeCb[0xfe]=16;

        if (!_memory.hasBootRom()) {
            AF(0x01B0);
            _BC.word = 0x0013;
            _DE.word = 0x00D8;
            _HL.word = 0x014D;
            m_SP = 0xFFFE;
            _PC = 0x100;
        }
        else {
            _PC = 0x00;
        }
    }

    unsigned char CPU::readPCByte()
    {
        JFX_ASSERT( _PC != 0xFFFF );
        return _memory.readByte( _PC++ );
    }

    char CPU::readPCSignedByte()
    {
        return static_cast< char >( readPCByte() );
    }

    unsigned short CPU::readPCWord()
    {
        const unsigned short wordRead = _memory.readWord( _PC );
        JFX_ASSERT( _PC < 0xFFFE );
        _PC+= 2;
        return wordRead;
    }

    Opcode CPU::decodeOpcode()
    {
        const unsigned char opcode( readPCByte() );
        if ( opcode == 0xCB ) {
            const unsigned char opcodeHigh( readPCByte() );
            return static_cast< Opcode >( opcode << 8 | opcodeHigh );
        }
        else {
            return static_cast< Opcode >( opcode );
        }
    }
    
    int CPU::previewInstructionTiming() const
    {
        const Opcode opcode = previewOpcode();
        if ( opcode > 256 ) {
            return _opTimeCb[ opcode & 0xFF ];
        }
        else {
            return _opTime[ opcode ];
        }
    }

    Opcode CPU::previewOpcode() const
    {
        const unsigned char opcode( _memory.readByte( _PC ) );
        if ( opcode == 0xCB ) {
            const unsigned char opcodeHigh( _memory.readByte( _PC + 1 ) );
            return static_cast< Opcode >( opcode << 8 | opcodeHigh );
        }
        else {
            return static_cast< Opcode >( opcode );
        }
    }

    int CPU::execute( Opcode opcode )
    {
        switch( opcode ) {
            case kNop: break;
            case kJP: JP_nn(); break;
            case HALT: halt(); break;

            case EI: ei(); break;
            case DI: di(); break;

            case RST_00: rst( 0x00 ); break;
            case RST_08: rst( 0x08 ); break;
            case RST_10: rst( 0x10 ); break;
            case RST_18: rst( 0x18 ); break;
            case RST_20: rst( 0x20 ); break;
            case RST_28: rst( 0x28 ); break;
            case RST_30: rst( 0x30 ); break;
            case RST_38: rst( 0x38 ); break;

            case DAA : daa(); break;
            case SCF: scf(); break;
            case CCF: ccf(); break;

            case kXOR_A: XOR_n( _A ); break;
            case kXOR_B: XOR_n( _BC.b ); break;
            case kXOR_C: XOR_n( _BC.c ); break;
            case kXOR_D: XOR_n( _DE.d ); break;
            case kXOR_E: XOR_n( _DE.e ); break;
            case kXOR_H: XOR_n( _HL.h ); break;
            case kXOR_L: XOR_n( _HL.l ); break;
            case kXOR_HL: XOR_n( _memory.readByte( _HL.word ) ); break;
            case kXOR_STAR: XOR_n( readPCByte() ); ; break;

            case LD_A_n: ld_r_n( _A ); break;
            case LD_B_n: ld_r_n( _BC.b ); break;
            case LD_C_n: ld_r_n( _BC.c ); break;
            case LD_D_n: ld_r_n( _DE.d ); break;
            case LD_E_n: ld_r_n( _DE.e ); break;
            case LD_H_n: ld_r_n( _HL.h ); break;
            case LD_L_n: ld_r_n( _HL.l ); break;

            case LD_BC_nn: ld_r_nn( _BC.word ); break;
            case LD_DE_nn: ld_r_nn( _DE.word ); break;
            case LD_HL_nn: ld_r_nn( _HL.word ); break;
            case LD_SP_nn: ld_r_nn( m_SP ); break;

            case LD_B_A: ld_n_a( _BC.b ); break;
            case LD_C_A: ld_n_a( _BC.c ); break;
            case LD_D_A: ld_n_a( _DE.d ); break;
            case LD_E_A: ld_n_a( _DE.e ); break;
            case LD_H_A: ld_n_a( _HL.h ); break;
            case LD_L_A: ld_n_a( _HL.l ); break;

            case LD_BC_A: ld_nn_a( _BC.word ); break;
            case LD_DE_A: ld_nn_a( _DE.word ); break;
            case LD_HL_A: ld_nn_a( _HL.word ); break;
            case LD_nn_A: ld_nn_a( readPCWord() ); break;

            case LD_nn_SP: ld_nn_sp(); break;
            case LD_SP_HL: ld_sp_hl(); break;

            case LDD_HL_A: ldd_hl_a(); break;
            case LDI_HL_A: ldi_hl_a(); break;
            case LDD_A_HL: ldd_a_hl(); break;
            case LDI_A_HL: ldi_a_hl(); break;

            case LD_HL_SP_n: ld_hl_sp_n(); break;

            case DEC_A: dec_n( _A ); break;
            case DEC_B: dec_n( _BC.b ); break;
            case DEC_C: dec_n( _BC.c ); break;
            case DEC_D: dec_n( _DE.d ); break;
            case DEC_E: dec_n( _DE.e ); break;
            case DEC_H: dec_n( _HL.h ); break;
            case DEC_L: dec_n( _HL.l ); break;
            case DEC_MHL: dec_MemoryHL(); break;

            case DEC_BC: dec_nn( _BC.word ); break;
            case DEC_DE: dec_nn( _DE.word ); break;
            case DEC_HL: dec_nn( _HL.word ); break;
            case DEC_SP: dec_nn( m_SP ); break;

            case INC_A: inc_n( _A ); break;
            case INC_B: inc_n( _BC.b ); break;
            case INC_C: inc_n( _BC.c ); break;
            case INC_D: inc_n( _DE.d ); break;
            case INC_E: inc_n( _DE.e ); break;
            case INC_H: inc_n( _HL.h ); break;
            case INC_L: inc_n( _HL.l ); break;
            case INC_MHL: inc_MemoryHL(); break;

            case INC_BC: inc_nn( _BC.word ); break;
            case INC_DE: inc_nn( _DE.word ); break;
            case INC_HL: inc_nn( _HL.word ); break;
            case INC_SP: inc_nn( m_SP ); break;

            case JR_NZ_n: jr_cc_n( !_zero ); break;
            case JR_Z_n: jr_cc_n( _zero ); break;
            case JR_NC_n: jr_cc_n( !_carry ); break;
            case JR_C_n: jr_cc_n( _carry ); break; 

            case JR_n: jr_n(); break; 

            case JP_MHL: jp_hl(); break;

            case JP_NZ_nn: jp_cc_nn( !_zero ); break;
            case JP_Z_nn:  jp_cc_nn( _zero ); break;
            case JP_NC_nn: jp_cc_nn( !_carry ); break;
            case JP_C_nn:  jp_cc_nn( _carry ); break;

            case OR_A: or_n( _A ); break;
            case OR_B: or_n( _BC.b ); break;
            case OR_C: or_n( _BC.c ); break;
            case OR_D: or_n( _DE.d ); break;
            case OR_E: or_n( _DE.e ); break;
            case OR_H: or_n( _HL.h ); break;
            case OR_L: or_n( _HL.l ); break;
            case OR_HL: or_n( _memory.readByte( _HL.word ) );break;
            case OR_STAR: or_n( readPCByte() ); break;

            case LD_A_A:  ld_r1_r2( _A, _A ); break; 
            case LD_A_B:  ld_r1_r2( _A, _BC.b ); break; 
            case LD_A_C:  ld_r1_r2( _A, _BC.c ); break; 
            case LD_A_D:  ld_r1_r2( _A, _DE.d ); break; 
            case LD_A_E:  ld_r1_r2( _A, _DE.e ); break; 
            case LD_A_H:  ld_r1_r2( _A, _HL.h ); break; 
            case LD_A_L:  ld_r1_r2( _A, _HL.l ); break; 

            case LD_A_HL: ld_r1_r2( _A, _memory.readByte( _HL.word ) ); break; 

            case LD_A_BC: ld_a_n( _memory.readByte( _BC.word ) ); break; 
            case LD_A_DE: ld_a_n( _memory.readByte( _DE.word ) ); break; 
            case LD_A_nn: ld_a_n( _memory.readByte( readPCWord() ) ); break; 

            case LD_B_B:  ld_r1_r2( _BC.b, _BC.b ); break; 
            case LD_B_C:  ld_r1_r2( _BC.b, _BC.c ); break; 
            case LD_B_D:  ld_r1_r2( _BC.b, _DE.d ); break; 
            case LD_B_E:  ld_r1_r2( _BC.b, _DE.e ); break; 
            case LD_B_H:  ld_r1_r2( _BC.b, _HL.h ); break; 
            case LD_B_L:  ld_r1_r2( _BC.b, _HL.l ); break; 
            case LD_B_HL: ld_r1_r2( _BC.b, _memory.readByte( _HL.word ) ); break;
            case LD_C_B:  ld_r1_r2( _BC.c, _BC.b ); break; 
            case LD_C_C:  ld_r1_r2( _BC.c, _BC.c ); break; 
            case LD_C_D:  ld_r1_r2( _BC.c, _DE.d ); break; 
            case LD_C_E:  ld_r1_r2( _BC.c, _DE.e ); break; 
            case LD_C_H:  ld_r1_r2( _BC.c, _HL.h ); break; 
            case LD_C_L:  ld_r1_r2( _BC.c, _HL.l ); break; 
            case LD_C_HL: ld_r1_r2( _BC.c, _memory.readByte( _HL.word ) ); break;
            case LD_D_B:  ld_r1_r2( _DE.d, _BC.b ); break; 
            case LD_D_C:  ld_r1_r2( _DE.d, _BC.c ); break; 
            case LD_D_D:  ld_r1_r2( _DE.d, _DE.d ); break; 
            case LD_D_E:  ld_r1_r2( _DE.d, _DE.e ); break; 
            case LD_D_H:  ld_r1_r2( _DE.d, _HL.h ); break; 
            case LD_D_L:  ld_r1_r2( _DE.d, _HL.l ); break; 
            case LD_D_HL: ld_r1_r2( _DE.d, _memory.readByte( _HL.word ) ); break;
            case LD_E_B:  ld_r1_r2( _DE.e, _BC.b ); break; 
            case LD_E_C:  ld_r1_r2( _DE.e, _BC.c ); break; 
            case LD_E_D:  ld_r1_r2( _DE.e, _DE.d ); break; 
            case LD_E_E:  ld_r1_r2( _DE.e, _DE.e ); break; 
            case LD_E_H:  ld_r1_r2( _DE.e, _HL.h ); break; 
            case LD_E_L:  ld_r1_r2( _DE.e, _HL.l ); break; 
            case LD_E_HL: ld_r1_r2( _DE.e, _memory.readByte( _HL.word ) ); break;
            case LD_H_B:  ld_r1_r2( _HL.h, _BC.b ); break; 
            case LD_H_C:  ld_r1_r2( _HL.h, _BC.c ); break; 
            case LD_H_D:  ld_r1_r2( _HL.h, _DE.d ); break; 
            case LD_H_E:  ld_r1_r2( _HL.h, _DE.e ); break; 
            case LD_H_H:  ld_r1_r2( _HL.h, _HL.h ); break; 
            case LD_H_L:  ld_r1_r2( _HL.h, _HL.l ); break; 
            case LD_H_HL: ld_r1_r2( _HL.h, _memory.readByte( _HL.word ) ); break;
            case LD_L_B:  ld_r1_r2( _HL.l, _BC.b ); break; 
            case LD_L_C:  ld_r1_r2( _HL.l, _BC.c ); break; 
            case LD_L_D:  ld_r1_r2( _HL.l, _DE.d ); break; 
            case LD_L_E:  ld_r1_r2( _HL.l, _DE.e ); break; 
            case LD_L_H:  ld_r1_r2( _HL.l, _HL.h ); break; 
            case LD_L_L:  ld_r1_r2( _HL.l, _HL.l ); break; 
            case LD_L_HL: ld_r1_r2( _HL.l, _memory.readByte( _HL.word ) ); break;
            case LD_HL_B: ld_hl_r2( _BC.b ); break;
            case LD_HL_C: ld_hl_r2( _BC.c ); break;
            case LD_HL_D: ld_hl_r2( _DE.d ); break;
            case LD_HL_E: ld_hl_r2( _DE.e ); break;
            case LD_HL_H: ld_hl_r2( _HL.h ); break;
            case LD_HL_L: ld_hl_r2( _HL.l ); break;
            case LD_HL_n: ld_hl_r2( readPCByte() ); break;

            case BIT_0_A:  bit_b_r( 0, _A ); break;
            case BIT_0_B:  bit_b_r( 0, _BC.b ); break;
            case BIT_0_C:  bit_b_r( 0, _BC.c ); break;
            case BIT_0_D:  bit_b_r( 0, _DE.d ); break;
            case BIT_0_E:  bit_b_r( 0, _DE.e ); break;
            case BIT_0_H:  bit_b_r( 0, _HL.h ); break;
            case BIT_0_L:  bit_b_r( 0, _HL.l ); break;
            case BIT_0_HL: bit_b_r( 0, _memory.readByte( _HL.word ) ); break;
            case BIT_1_A:  bit_b_r( 1, _A ); break;
            case BIT_1_B:  bit_b_r( 1, _BC.b ); break;
            case BIT_1_C:  bit_b_r( 1, _BC.c ); break;
            case BIT_1_D:  bit_b_r( 1, _DE.d ); break;
            case BIT_1_E:  bit_b_r( 1, _DE.e ); break;
            case BIT_1_H:  bit_b_r( 1, _HL.h ); break;
            case BIT_1_L:  bit_b_r( 1, _HL.l ); break;
            case BIT_1_HL: bit_b_r( 1, _memory.readByte( _HL.word ) ); break;
            case BIT_2_A:  bit_b_r( 2, _A ); break;
            case BIT_2_B:  bit_b_r( 2, _BC.b ); break;
            case BIT_2_C:  bit_b_r( 2, _BC.c ); break;
            case BIT_2_D:  bit_b_r( 2, _DE.d ); break;
            case BIT_2_E:  bit_b_r( 2, _DE.e ); break;
            case BIT_2_H:  bit_b_r( 2, _HL.h ); break;
            case BIT_2_L:  bit_b_r( 2, _HL.l ); break;
            case BIT_2_HL: bit_b_r( 2, _memory.readByte( _HL.word ) ); break;
            case BIT_3_A:  bit_b_r( 3, _A ); break;
            case BIT_3_B:  bit_b_r( 3, _BC.b ); break;
            case BIT_3_C:  bit_b_r( 3, _BC.c ); break;
            case BIT_3_D:  bit_b_r( 3, _DE.d ); break;
            case BIT_3_E:  bit_b_r( 3, _DE.e ); break;
            case BIT_3_H:  bit_b_r( 3, _HL.h ); break;
            case BIT_3_L:  bit_b_r( 3, _HL.l ); break;
            case BIT_3_HL: bit_b_r( 3, _memory.readByte( _HL.word ) ); break;
            case BIT_4_A:  bit_b_r( 4, _A ); break;
            case BIT_4_B:  bit_b_r( 4, _BC.b ); break;
            case BIT_4_C:  bit_b_r( 4, _BC.c ); break;
            case BIT_4_D:  bit_b_r( 4, _DE.d ); break;
            case BIT_4_E:  bit_b_r( 4, _DE.e ); break;
            case BIT_4_H:  bit_b_r( 4, _HL.h ); break;
            case BIT_4_L:  bit_b_r( 4, _HL.l ); break;
            case BIT_4_HL: bit_b_r( 4, _memory.readByte( _HL.word ) ); break;
            case BIT_5_A:  bit_b_r( 5, _A ); break;
            case BIT_5_B:  bit_b_r( 5, _BC.b ); break;
            case BIT_5_C:  bit_b_r( 5, _BC.c ); break;
            case BIT_5_D:  bit_b_r( 5, _DE.d ); break;
            case BIT_5_E:  bit_b_r( 5, _DE.e ); break;
            case BIT_5_H:  bit_b_r( 5, _HL.h ); break;
            case BIT_5_L:  bit_b_r( 5, _HL.l ); break;
            case BIT_5_HL: bit_b_r( 5, _memory.readByte( _HL.word ) ); break;
            case BIT_6_A:  bit_b_r( 6, _A ); break;
            case BIT_6_B:  bit_b_r( 6, _BC.b ); break;
            case BIT_6_C:  bit_b_r( 6, _BC.c ); break;
            case BIT_6_D:  bit_b_r( 6, _DE.d ); break;
            case BIT_6_E:  bit_b_r( 6, _DE.e ); break;
            case BIT_6_H:  bit_b_r( 6, _HL.h ); break;
            case BIT_6_L:  bit_b_r( 6, _HL.l ); break;
            case BIT_6_HL: bit_b_r( 6, _memory.readByte( _HL.word ) ); break;
            case BIT_7_A:  bit_b_r( 7, _A ); break;
            case BIT_7_B:  bit_b_r( 7, _BC.b ); break;
            case BIT_7_C:  bit_b_r( 7, _BC.c ); break;
            case BIT_7_D:  bit_b_r( 7, _DE.d ); break;
            case BIT_7_E:  bit_b_r( 7, _DE.e ); break;
            case BIT_7_H:  bit_b_r( 7, _HL.h ); break;
            case BIT_7_L:  bit_b_r( 7, _HL.l ); break;
            case BIT_7_HL: bit_b_r( 7, _memory.readByte( _HL.word ) ); break;

            case RES_0_A:  res_b_r( 0, _A ); break;
            case RES_0_B:  res_b_r( 0, _BC.b ); break;
            case RES_0_C:  res_b_r( 0, _BC.c ); break;
            case RES_0_D:  res_b_r( 0, _DE.d ); break;
            case RES_0_E:  res_b_r( 0, _DE.e ); break;
            case RES_0_H:  res_b_r( 0, _HL.h ); break;
            case RES_0_L:  res_b_r( 0, _HL.l ); break;
            case RES_0_HL: res_b_mhl( 0 ); break;
            case RES_1_A:  res_b_r( 1, _A ); break;
            case RES_1_B:  res_b_r( 1, _BC.b ); break;
            case RES_1_C:  res_b_r( 1, _BC.c ); break;
            case RES_1_D:  res_b_r( 1, _DE.d ); break;
            case RES_1_E:  res_b_r( 1, _DE.e ); break;
            case RES_1_H:  res_b_r( 1, _HL.h ); break;
            case RES_1_L:  res_b_r( 1, _HL.l ); break;
            case RES_1_HL: res_b_mhl( 1 ); break;
            case RES_2_A:  res_b_r( 2, _A ); break;
            case RES_2_B:  res_b_r( 2, _BC.b ); break;
            case RES_2_C:  res_b_r( 2, _BC.c ); break;
            case RES_2_D:  res_b_r( 2, _DE.d ); break;
            case RES_2_E:  res_b_r( 2, _DE.e ); break;
            case RES_2_H:  res_b_r( 2, _HL.h ); break;
            case RES_2_L:  res_b_r( 2, _HL.l ); break;
            case RES_2_HL: res_b_mhl( 2 ); break;
            case RES_3_A:  res_b_r( 3, _A ); break;
            case RES_3_B:  res_b_r( 3, _BC.b ); break;
            case RES_3_C:  res_b_r( 3, _BC.c ); break;
            case RES_3_D:  res_b_r( 3, _DE.d ); break;
            case RES_3_E:  res_b_r( 3, _DE.e ); break;
            case RES_3_H:  res_b_r( 3, _HL.h ); break;
            case RES_3_L:  res_b_r( 3, _HL.l ); break;
            case RES_3_HL: res_b_mhl( 3 ); break;
            case RES_4_A:  res_b_r( 4, _A ); break;
            case RES_4_B:  res_b_r( 4, _BC.b ); break;
            case RES_4_C:  res_b_r( 4, _BC.c ); break;
            case RES_4_D:  res_b_r( 4, _DE.d ); break;
            case RES_4_E:  res_b_r( 4, _DE.e ); break;
            case RES_4_H:  res_b_r( 4, _HL.h ); break;
            case RES_4_L:  res_b_r( 4, _HL.l ); break;
            case RES_4_HL: res_b_mhl( 4 ); break;
            case RES_5_A:  res_b_r( 5, _A ); break;
            case RES_5_B:  res_b_r( 5, _BC.b ); break;
            case RES_5_C:  res_b_r( 5, _BC.c ); break;
            case RES_5_D:  res_b_r( 5, _DE.d ); break;
            case RES_5_E:  res_b_r( 5, _DE.e ); break;
            case RES_5_H:  res_b_r( 5, _HL.h ); break;
            case RES_5_L:  res_b_r( 5, _HL.l ); break;
            case RES_5_HL: res_b_mhl( 5 ); break;
            case RES_6_A:  res_b_r( 6, _A ); break;
            case RES_6_B:  res_b_r( 6, _BC.b ); break;
            case RES_6_C:  res_b_r( 6, _BC.c ); break;
            case RES_6_D:  res_b_r( 6, _DE.d ); break;
            case RES_6_E:  res_b_r( 6, _DE.e ); break;
            case RES_6_H:  res_b_r( 6, _HL.h ); break;
            case RES_6_L:  res_b_r( 6, _HL.l ); break;
            case RES_6_HL: res_b_mhl( 6 ); break;
            case RES_7_A:  res_b_r( 7, _A ); break;
            case RES_7_B:  res_b_r( 7, _BC.b ); break;
            case RES_7_C:  res_b_r( 7, _BC.c ); break;
            case RES_7_D:  res_b_r( 7, _DE.d ); break;
            case RES_7_E:  res_b_r( 7, _DE.e ); break;
            case RES_7_H:  res_b_r( 7, _HL.h ); break;
            case RES_7_L:  res_b_r( 7, _HL.l ); break;
            case RES_7_HL: res_b_mhl( 7 ); break;

            case SET_0_A:  set_b_r( 0, _A ); break;
            case SET_0_B:  set_b_r( 0, _BC.b ); break;
            case SET_0_C:  set_b_r( 0, _BC.c ); break;
            case SET_0_D:  set_b_r( 0, _DE.d ); break;
            case SET_0_E:  set_b_r( 0, _DE.e ); break;
            case SET_0_H:  set_b_r( 0, _HL.h ); break;
            case SET_0_L:  set_b_r( 0, _HL.l ); break;
            case SET_0_HL: set_b_mhl( 0 ); break;
            case SET_1_A:  set_b_r( 1, _A ); break;
            case SET_1_B:  set_b_r( 1, _BC.b ); break;
            case SET_1_C:  set_b_r( 1, _BC.c ); break;
            case SET_1_D:  set_b_r( 1, _DE.d ); break;
            case SET_1_E:  set_b_r( 1, _DE.e ); break;
            case SET_1_H:  set_b_r( 1, _HL.h ); break;
            case SET_1_L:  set_b_r( 1, _HL.l ); break;
            case SET_1_HL: set_b_mhl( 1 ); break;
            case SET_2_A:  set_b_r( 2, _A ); break;
            case SET_2_B:  set_b_r( 2, _BC.b ); break;
            case SET_2_C:  set_b_r( 2, _BC.c ); break;
            case SET_2_D:  set_b_r( 2, _DE.d ); break;
            case SET_2_E:  set_b_r( 2, _DE.e ); break;
            case SET_2_H:  set_b_r( 2, _HL.h ); break;
            case SET_2_L:  set_b_r( 2, _HL.l ); break;
            case SET_2_HL: set_b_mhl( 2 ); break;
            case SET_3_A:  set_b_r( 3, _A ); break;
            case SET_3_B:  set_b_r( 3, _BC.b ); break;
            case SET_3_C:  set_b_r( 3, _BC.c ); break;
            case SET_3_D:  set_b_r( 3, _DE.d ); break;
            case SET_3_E:  set_b_r( 3, _DE.e ); break;
            case SET_3_H:  set_b_r( 3, _HL.h ); break;
            case SET_3_L:  set_b_r( 3, _HL.l ); break;
            case SET_3_HL: set_b_mhl( 3 ); break;
            case SET_4_A:  set_b_r( 4, _A ); break;
            case SET_4_B:  set_b_r( 4, _BC.b ); break;
            case SET_4_C:  set_b_r( 4, _BC.c ); break;
            case SET_4_D:  set_b_r( 4, _DE.d ); break;
            case SET_4_E:  set_b_r( 4, _DE.e ); break;
            case SET_4_H:  set_b_r( 4, _HL.h ); break;
            case SET_4_L:  set_b_r( 4, _HL.l ); break;
            case SET_4_HL: set_b_mhl( 4 ); break;
            case SET_5_A:  set_b_r( 5, _A ); break;
            case SET_5_B:  set_b_r( 5, _BC.b ); break;
            case SET_5_C:  set_b_r( 5, _BC.c ); break;
            case SET_5_D:  set_b_r( 5, _DE.d ); break;
            case SET_5_E:  set_b_r( 5, _DE.e ); break;
            case SET_5_H:  set_b_r( 5, _HL.h ); break;
            case SET_5_L:  set_b_r( 5, _HL.l ); break;
            case SET_5_HL: set_b_mhl( 5 ); break;
            case SET_6_A:  set_b_r( 6, _A ); break;
            case SET_6_B:  set_b_r( 6, _BC.b ); break;
            case SET_6_C:  set_b_r( 6, _BC.c ); break;
            case SET_6_D:  set_b_r( 6, _DE.d ); break;
            case SET_6_E:  set_b_r( 6, _DE.e ); break;
            case SET_6_H:  set_b_r( 6, _HL.h ); break;
            case SET_6_L:  set_b_r( 6, _HL.l ); break;
            case SET_6_HL: set_b_mhl( 6 ); break;
            case SET_7_A:  set_b_r( 7, _A ); break;
            case SET_7_B:  set_b_r( 7, _BC.b ); break;
            case SET_7_C:  set_b_r( 7, _BC.c ); break;
            case SET_7_D:  set_b_r( 7, _DE.d ); break;
            case SET_7_E:  set_b_r( 7, _DE.e ); break;
            case SET_7_H:  set_b_r( 7, _HL.h ); break;
            case SET_7_L:  set_b_r( 7, _HL.l ); break;
            case SET_7_HL: set_b_mhl( 7 ); break;

            case LD_FF00_C_A: ld_ff00_c_a( ); break;
            case LD_A_FF00_C: ld_a_ff00_c(); break;
            case LDH_n_A: ldh_n_a(); break;
            case LDH_A_n: ldh_a_n(); break;

            case CALL_nn: call_nn(); break;
            
            case CALL_NZ_nn: call_cc_nn( !_zero ); break;
            case CALL_Z_nn: call_cc_nn( _zero ); break;
            case CALL_NC_nn: call_cc_nn( !_carry ); break;
            case CALL_C_nn: call_cc_nn( _carry ); break;

            case RET: ret(); break;

            case RETI: reti(); break;

            case RET_NZ: ret_cc( !_zero ); break;
            case RET_Z:  ret_cc( _zero ); break;
            case RET_NC: ret_cc( !_carry ); break;
            case RET_C:  ret_cc( _carry ); break;

            case PUSH_AF: push_nn( AF() ); break;
            case PUSH_BC: push_nn( _BC.word ); break;
            case PUSH_DE: push_nn( _DE.word ); break;
            case PUSH_HL: push_nn( _HL.word ); break;

            case POP_AF: pop_af(); break;
            case POP_BC: pop_nn( _BC.word ); break;
            case POP_DE: pop_nn( _DE.word ); break;
            case POP_HL: pop_nn( _HL.word ); break;

            case RRCA: rrc_n( _A, false ); break; 
            case RRC_A: rrc_n( _A ); break;
            case RRC_B: rrc_n( _BC.b ); break;
            case RRC_C: rrc_n( _BC.c ); break;
            case RRC_D: rrc_n( _DE.d ); break;
            case RRC_E: rrc_n( _DE.e ); break;
            case RRC_H: rrc_n( _HL.h ); break;
            case RRC_L: rrc_n( _HL.l ); break;
            case RRC_MHL: rrc_mhl(); break;

            case RRA: rr_n( _A, false ); break;
            case RR_A: rr_n( _A ); break;
            case RR_B: rr_n( _BC.b ); break;
            case RR_C: rr_n( _BC.c ); break;
            case RR_D: rr_n( _DE.d ); break;
            case RR_E: rr_n( _DE.e ); break;
            case RR_H: rr_n( _HL.h ); break;
            case RR_L: rr_n( _HL.l ); break;
            case RR_HL: rr_hl(); break;

            case RLA: rl_n( _A, false ); break;
            case RL_A: rl_n( _A ); break;
            case RL_B: rl_n( _BC.b ); break;
            case RL_C: rl_n( _BC.c ); break;
            case RL_D: rl_n( _DE.d ); break;
            case RL_E: rl_n( _DE.e ); break;
            case RL_H: rl_n( _HL.h ); break;
            case RL_L: rl_n( _HL.l ); break;
            case RL_HL: rl_mhl(); break;

            case RLCA: rlc_n( _A, false ); break;
            case RLC_A: rlc_n( _A ); break; 
            case RLC_B: rlc_n( _BC.b ); break; 
            case RLC_C: rlc_n( _BC.c ); break; 
            case RLC_D: rlc_n( _DE.d ); break; 
            case RLC_E: rlc_n( _DE.e ); break; 
            case RLC_H: rlc_n( _HL.h ); break; 
            case RLC_L: rlc_n( _HL.l ); break; 
            case RLC_MHL: rlc_mhl(); break; 

            case SRL_A: srl_n( _A ); break;
            case SRL_B: srl_n( _BC.b ); break;
            case SRL_C: srl_n( _BC.c ); break;
            case SRL_D: srl_n( _DE.d ); break;
            case SRL_E: srl_n( _DE.e ); break;
            case SRL_H: srl_n( _HL.h ); break;
            case SRL_L: srl_n( _HL.l ); break;
            case SRL_MHL: srl_mhl(); break;

            case SLA_A: sla_n( _A ); break;
            case SLA_B: sla_n( _BC.b ); break;
            case SLA_C: sla_n( _BC.c ); break;
            case SLA_D: sla_n( _DE.d ); break;
            case SLA_E: sla_n( _DE.e ); break;
            case SLA_H: sla_n( _HL.h ); break;
            case SLA_L: sla_n( _HL.l ); break;
            case SLA_MHL: sla_mhl(); break;

            case SRA_A: sra_n( _A ); break;
            case SRA_B: sra_n( _BC.b ); break;
            case SRA_C: sra_n( _BC.c ); break;
            case SRA_D: sra_n( _DE.d ); break;
            case SRA_E: sra_n( _DE.e ); break;
            case SRA_H: sra_n( _HL.h ); break;
            case SRA_L: sra_n( _HL.l ); break;
            case SRA_MHL: sra_mhl(); break;

            case CPL: cpl(); break;

            case CP_A: cp_n( _A ); break;
            case CP_B: cp_n( _BC.b ); break;
            case CP_C: cp_n( _BC.c ); break;
            case CP_D: cp_n( _DE.d ); break;
            case CP_E: cp_n( _DE.e ); break;
            case CP_H: cp_n( _HL.h ); break;
            case CP_L: cp_n( _HL.l ); break;
            case CP_HL: cp_n( _memory.readByte( _HL.word ) ); break;
            case CP_N: cp_n( readPCByte() ); break;

            case AND_A: and_n( _A ); break; 
            case AND_B: and_n( _BC.b ); break;
            case AND_C: and_n( _BC.c ); break;
            case AND_D: and_n( _DE.d ); break;
            case AND_E: and_n( _DE.e ); break;
            case AND_H: and_n( _HL.h ); break;
            case AND_L: and_n( _HL.l ); break;
            case AND_MHL: and_n( _memory.readByte( _HL.word ) ); break;
            case AND_PC: and_n( readPCByte() ); break;


            case SWAP_A: swap_n( _A ); break;
            case SWAP_B: swap_n( _BC.b ); break;
            case SWAP_C: swap_n( _BC.c ); break;
            case SWAP_D: swap_n( _DE.d ); break;
            case SWAP_E: swap_n( _DE.e ); break;
            case SWAP_H: swap_n( _HL.h ); break;
            case SWAP_L: swap_n( _HL.l ); break;
            case SWAP_MHL: swap_mhl(); break;

            case ADD_A: add_n( _A ); break; 
            case ADD_B: add_n( _BC.b ); break; 
            case ADD_C: add_n( _BC.c ); break; 
            case ADD_D: add_n( _DE.d ); break; 
            case ADD_E: add_n( _DE.e ); break; 
            case ADD_H: add_n( _HL.h ); break; 
            case ADD_L: add_n( _HL.l ); break; 
            case ADD_MHL: add_n( _memory.readByte( _HL.word ) ); break;
            case ADD_N: add_n( readPCByte() ); break;

            case ADD_SP_n: add_sp_n(); break;

            case ADD_HL_BC: add_hl_n( _BC.word ); break;
            case ADD_HL_DE: add_hl_n( _DE.word ); break;
            case ADD_HL_HL: add_hl_n( _HL.word ); break;
            case ADD_HL_SP: add_hl_n( m_SP ); break;

            case ACD_A_A: adc_a_n( _A ); break;
            case ACD_A_B: adc_a_n( _BC.b ); break;
            case ACD_A_C: adc_a_n( _BC.c ); break;
            case ACD_A_D: adc_a_n( _DE.d ); break;
            case ACD_A_E: adc_a_n( _DE.e ); break;
            case ACD_A_H: adc_a_n( _HL.h ); break;
            case ACD_A_L: adc_a_n( _HL.l ); break;
            case ACD_A_MHL: adc_a_n( _memory.readByte( _HL.word ) ); break;
            case ACD_A_MPC: adc_a_n( readPCByte() ); break;

            case SBC_A_A: sbc_a_n( _A ); break;
            case SBC_A_B: sbc_a_n( _BC.b ); break;
            case SBC_A_C: sbc_a_n( _BC.c ); break;
            case SBC_A_D: sbc_a_n( _DE.d ); break;
            case SBC_A_E: sbc_a_n( _DE.e ); break;
            case SBC_A_H: sbc_a_n( _HL.h ); break;
            case SBC_A_L: sbc_a_n( _HL.l ); break;
            case SBC_A_MHL: sbc_a_n( _memory.readByte( _HL.word ) ); break;
            case SBC_A_n: sbc_a_n( readPCByte() ); break;

            case SUB_A: sub_n( _A ); break; 
            case SUB_B: sub_n( _BC.b ); break; 
            case SUB_C: sub_n( _BC.c ); break; 
            case SUB_D: sub_n( _DE.d ); break; 
            case SUB_E: sub_n( _DE.e ); break; 
            case SUB_H: sub_n( _HL.h ); break; 
            case SUB_L: sub_n( _HL.l ); break; 
            case SUB_MHL: sub_n( _memory.readByte( _HL.word ) ); break;
            case SUB_N: sub_n( readPCByte() ); break;

            default: {
                std::cout << "Unrecognized opcode : " << std::hex << static_cast< unsigned short >( opcode ) << " PC = " << _PC << std::endl;
                return -1;
            } break;
        };
        if ( isCBOpcode( opcode ) ) {
            return _opTimeCb[ opcode & 0xFF ];
        }
        else {
            return _opTime[ opcode ];
        }
    }

    void CPU::JP_nn() 
    {
        _PC = readPCWord();
    }

    void CPU::XOR_n( const unsigned char value )
    {
        _A ^= value;
        _zero = _A == 0;
        _carry = _halfCarry = _substract = false;
    }

    void CPU::ld_r_n( unsigned char& reg )
    {
        reg = readPCByte();
    }
        
    void CPU::ld_r_nn( unsigned short& reg )
    {
        reg = readPCWord();
    }

    void CPU::ld_n_a( unsigned char& reg )
    {
        reg = _A;
    }

    void CPU::ld_nn_a( unsigned short addr )
    {
        _memory.writeByte( addr, _A );
    }

    void CPU::ld_nn_sp()
    {
        _memory.writeWord( readPCWord(), m_SP );
    }

    void CPU::ld_sp_hl()
    {
        m_SP = _HL.word;
    }

    void CPU::ldd_hl_a()
    {
        // load A at HL
        _memory.writeByte( _HL.word, _A );
        // decrement HL
        --_HL.word;
    }

    void CPU::ldi_hl_a()
    {
        // load A at HL
        _memory.writeByte( _HL.word, _A );
        // increment HL
        ++_HL.word;
    }

    void CPU::ldi_a_hl()
    {
        _A = _memory.readByte( _HL.word );
        ++_HL.word;
    }

    void CPU::ldd_a_hl()
    {
        _A = _memory.readByte( _HL.word );
        --_HL.word;
    }

    void CPU::dec_n( unsigned char& value )
    {
        _halfCarry = lowNibble( value ) == 0;
        --value;
        _zero = value == 0;
        _substract = true;
    }

    void CPU::dec_nn( unsigned short& value )
    {
        --value;
    }

    void CPU::dec_MemoryHL()
    {
        unsigned char value( _memory.readByte( _HL.word ) );
        dec_n( value );
        _memory.writeByte( _HL.word, value );
    }

    void CPU::jr_cc_n( bool flag )
    {
        const char offset( readPCSignedByte() );
        if ( flag ) {
            _PC += offset;
        }
    }

    void CPU::jr_n()
    {
        _PC += readPCSignedByte();
    }

    void CPU::jp_hl()
    {
        _PC = _HL.word;
    }

    void CPU::jp_cc_nn( bool flag )
    {
        const unsigned short jumpTo( readPCWord() );
        if ( flag ) {
            _PC = jumpTo;
        }
    }

    void CPU::or_n( unsigned char value )
    {
        _A |= value;
        _zero = _A == 0;
        _carry = false;
        _halfCarry = false;
        _substract = false;
    }

    void CPU::inc_n( unsigned char& value )
    {
        const bool carry = _carry;
        value = addImp( value, 1 );
        _carry = carry;
    }

    void CPU::inc_nn( unsigned short& reg )
    {
        ++reg;
    }

    void CPU::inc_MemoryHL()
    {
        unsigned char value = _memory.readByte( _HL.word );
        inc_n( value );
        _memory.writeByte( _HL.word, value );
    }

    void CPU::ld_r1_r2( 
        unsigned char& r1, unsigned char value 
    )
    {
        r1 = value;
    }

    void CPU::ld_a_n( unsigned char value )
    {
        _A = value;
    }
    
    void CPU::ld_hl_r2( unsigned char value )
    {
        _memory.writeByte( _HL.word, value );
    }

    void CPU::bit_b_r( unsigned int index, unsigned char value )
    {
        _zero = getBit( value, index ) == false;
        _substract = false;
        _halfCarry = true;
    }

    void CPU::res_b_r( unsigned int index, unsigned char &reg )
    {
        resetBit( reg, index );
    }

    void CPU::res_b_mhl( unsigned int index )
    {
        unsigned char value = _memory.readByte( _HL.word );
        res_b_r( index, value );
        _memory.writeByte( _HL.word, value );
    }

    void CPU::set_b_r( unsigned int index, unsigned char &reg )
    {
        setBit( reg, index );
    }

    void CPU::set_b_mhl( unsigned int index )
    {
        unsigned char value = _memory.readByte( _HL.word );
        set_b_r( index, value );
        _memory.writeByte( _HL.word, value );
    }

    void CPU::ld_ff00_c_a()
    {
        _memory.writeByte( 0xFF00 + _BC.c, _A );
    }

    void CPU::ld_a_ff00_c()
    {
        _A = _memory.readByte( 0xFF00 + _BC.c );
    }

    void CPU::ldh_n_a()
    {
        _memory.writeByte( 0xFF00 + readPCByte(), _A );
    }

    void CPU::ldh_a_n()
    {
        _A = _memory.readByte( 0xFF00 + readPCByte() );
    }

    void CPU::call_nn()
    {
        const unsigned short jumpTo( readPCWord() );
        push_nn( _PC );
        _PC = jumpTo;
    }

    void CPU::call_cc_nn( bool flag )
    {
        const unsigned short jumpTo( readPCWord() );
        if ( flag ) {
            push_nn( _PC );
            _PC = jumpTo;
        }
    }

    void CPU::ret()
    {
        pop_nn( _PC );
    }

    void CPU::reti()
    {
        ret();
        _interruptState = kEnabled;
    }   

    void CPU::ret_cc( bool flag )
    {
        if ( flag ) {
            ret();
        }
    }

    void CPU::push_nn( unsigned short value )
    {
        m_SP -= 2;
        _memory.writeWord( m_SP, value );
    }

    void CPU::pop_af()
    {
        unsigned short af;
        pop_nn( af );
        AF( af );
    }

    void CPU::pop_nn( unsigned short& value )
    {
        value = _memory.readWord( m_SP  );
        m_SP += 2;
    }

    void CPU::rrc_n( unsigned char& reg, bool zeroFlagMask )
    {
        _carry = getBit( reg, 0 );
        reg >>= 1;
        copyBit( reg, 7, _carry );

        _zero = ( reg == 0 ) & zeroFlagMask;
        _substract = _halfCarry = false;
    }
    void CPU::rrc_mhl()
    {
        unsigned char value = _memory.readByte( _HL.word );
        rrc_n( value );
        _memory.writeByte( _HL.word, value );
    }

    void CPU::rr_n( unsigned char& value, bool zeroFlagMask )
    {
        const unsigned char oldCarry( _carry ? 1 : 0 );
        _carry = ( value & 0x1 ) == 0x1;
        value >>= 1;
        value |= ( oldCarry << 7 );
        _zero = ( value == 0 ) & zeroFlagMask;
        _substract = false;
        _halfCarry = false;
    }

    void CPU::rr_hl()
    {
        unsigned char mem( _memory.readByte( _HL.word ) );
        rr_n( mem );
        _memory.writeByte( _HL.word, mem );
    }

    void CPU::rl_n( unsigned char& value, bool zeroFlagMask )
    {
        const unsigned char oldCarry( _carry ? 1 : 0 );
        _carry = ( value > 0x7f );
        value <<= 1;
        value |= oldCarry;
        _zero = ( value == 0 ) & zeroFlagMask;
        _substract = false;
        _halfCarry = false;
    }

    void CPU::rl_mhl()
    {
        unsigned char mem( _memory.readByte( _HL.word ) );
        rl_n( mem );
        _memory.writeByte( _HL.word, mem );
    }

    void CPU::rlc_n( unsigned char& value, bool zeroFlagMask )
    {
        _carry = getBit( value, 7 );
        value <<= 1;
        copyBit( value, 0, _carry );
        _substract = false;
        _halfCarry = false;
        _zero = ( value == 0 ) & zeroFlagMask;
    }
    
    void CPU::rlc_mhl()
    {
        unsigned char mem( _memory.readByte( _HL.word ) );
        rlc_n( mem );
        _memory.writeByte( _HL.word, mem );
    }

    void CPU::srl_n( unsigned char& reg )
    {
        _carry = ( reg & 0x1 ) == 0x1;
        reg >>= 1;
        _halfCarry = false;
        _substract = false;
        _zero = reg == 0;
    }

    void CPU::srl_mhl()
    {
        unsigned char value = _memory.readByte( _HL.word );
        srl_n( value );
        _memory.writeByte( _HL.word, value );
    }

    void CPU::sla_n( unsigned char& reg )
    {
        _carry = getMSB( reg );
        _halfCarry = _substract = false;
        reg <<= 1;
        _zero = ( reg == 0 );
    }

    void CPU::sla_mhl()
    {
        unsigned char mem = _memory.readByte( _HL.word );
        sla_n( mem );
        _memory.writeByte( _HL.word, mem );
    }

    void CPU::sra_n( unsigned char& reg )
    {
        _carry = getLSB( reg );

        const bool msb( getMSB( reg ) );
        reg >>= 1;
        copyBit( reg, 7, msb );

        _zero = ( reg == 0 );
        _halfCarry = _substract = false;
    }

    void CPU::sra_mhl()
    {
        unsigned char mem = _memory.readByte( _HL.word );
        sra_n( mem );
        _memory.writeByte( _HL.word, mem );
    }

    void CPU::cp_n( unsigned char value )
    {
        subImp( _A, value );
    }

    void CPU::and_n( unsigned char value )
    {
        _A = _A & value;
        _halfCarry = true;
        _substract = _carry = false;
        _zero = ( _A == 0 );
    }

    void CPU::swap_n( unsigned char& reg )
    {
        reg = (unsigned char)(( ( reg & 0x0F ) << 4 ) + ( ( reg & 0xF0 ) >> 4 ));
        _zero = ( reg == 0 );
        _substract = _halfCarry = _carry = false;
    }

    void CPU::swap_mhl()
    {
        unsigned char value( _memory.readByte( _HL.word ) );
        swap_n( value );
        _memory.writeByte( _HL.word, value );
    }

    void CPU::rst( unsigned short offset )
    {
        push_nn( _PC );
        _PC = offset;
    }

    void CPU::daa()
    {
        // Taken from code that was inspired by blarg's.
        int a = _A;
        if ( !_substract ) {
            if ( _halfCarry || lowNibble( a ) > 9) {
                a += 0x06;
            }
            if (_carry || a > 0x9f ) {
                a += 0x60;
            }
        }
        else {
            if (_halfCarry ) {
                a = ( a - 6 ) & 0xff;
            }
            if (_carry ) {
                a -= 0x60;
            }
        }

        _halfCarry = false;

        if ( a >= 256 ) {
            _carry = true;
        }

        _A = static_cast< unsigned char >( a );

        _zero = ( _A == 0 );
    }

    void CPU::scf()
    {
        _carry = true;
        _substract = _halfCarry = false;
    }

    void CPU::ccf()
    {
        _carry = !_carry;
        _substract = _halfCarry = false;
    }

    void CPU::cpl()
    {
        _substract = true;
        _halfCarry = true;
        _A = ~_A;
    }

    void CPU::sub_n( unsigned char value )
    {
        _A = subImp( _A, value );
    }

    void CPU::add_n( unsigned char value )
    {
        _A = addImp( _A, value );
    }

    void CPU::add_sp_n()
    {
        //m_SP = addWordImp( m_SP, readPCSignedByte(), false );
        _zero = false;
        _substract = false;
        char readByte = readPCSignedByte();
        /*
        FIXME : Implement half carry and carry
        if ( readByte >= 0 ) {
            _halfCarry = ( ( m_SP & 0x0f ) + lowNibble( readByte ) ) > 0x0F;
        }
        else {
            _halfCarry = lowNibble( m_SP ) < lowNibble( -readByte );
        }
        if ( readByte >= 0 ) {
            _carry = ( m_SP + readByte ) > 0xffff;
        }
        else {
            _carry = m_SP < -readByte;
        }*/
        m_SP += readByte;
    }

    void CPU::ld_hl_sp_n()
    {
        char readByte = readPCSignedByte();
        _zero = false;
        _substract = false;
        _halfCarry = (((m_SP & 0x0fff) + readByte) > 0x0fff) ? 1 : 0;
        _carry = ((m_SP + readByte) > 0xffff) ? 1 : 0;
        _HL.word = (unsigned short)(m_SP + readByte);
    }

    void CPU::adc_a_n( unsigned char value )
    {
        _A = addImp( _A, value, _carry );
    }

    void CPU::sbc_a_n( unsigned char value )
    {
        _A = subImp( _A, value, _carry );
    }

    void CPU::add_hl_n( unsigned short value )
    {
        _HL.word = addWordImp( _HL.word, value );
    }

    void CPU::di()
    {
        _interruptState = kDisabledNextCycle;
    }

    void CPU::ei()
    {
        _interruptState = kEnabledNextCycle;
    }

    void CPU::halt()
    {
        if ( _interruptState == kEnabled ) {
            _isHalted = true;
        }
    }

    unsigned char CPU::addImp( unsigned char left, unsigned char right, bool carry )
    {
        const unsigned char result = left + right + ( carry ? 1 : 0 );

        _zero = result == 0;
        _substract = false;
        _halfCarry = ( ( ( left & 0x0F ) + ( right & 0x0F ) + ( carry ? 1: 0 ) ) > 0x0F );
        _carry = ( left + right + ( carry ? 1 : 0 ) ) > 0xff;

        return result;
    }

    unsigned short CPU::addWordImp( unsigned short left, unsigned short right )
    {
        const unsigned short result = left + right;

        _substract = false;
        _halfCarry = ( ( left & 0x0fff ) + ( right & 0x0fff) ) > 0x0fff;
        _carry = ( ( left + right ) > 0xFFFF );

        return result;
    }

    unsigned char CPU::subImp( unsigned char left, unsigned char right, bool carry )
    {
        _substract = true;
        const int result = left - ( right + ( carry ? 1 : 0 ) );
        _carry = result < 0;
        _halfCarry = lowNibble(left) - lowNibble(right) < ( carry ? 1 : 0 );
        _zero = ( result & 0xff ) == 0;
        return result & 0xff;
    }

    bool CPU::areInterruptsEnabled() const
    {
        switch( _interruptState ) {
            case kDisabled:
            case kEnabledNextCycle:
                return false;
            case kEnabled:
            case kDisabledNextCycle:
                return true;
            default:
                JFX_MSG_ASSERT( "Wrong interrupt state : " << _interruptState );
                return false;
        };
    }

    void CPU::executeInterrupt( unsigned short interruptAddr )
    {
        _isHalted = false;
        // push the current address
        push_nn( _PC );
        // disable interrupts
        _interruptState = kDisabled;
        _PC = interruptAddr;
    }

    int CPU::emulateCycle()
    {
        // If previous instruction was di, disable interrupts after this instruction
        InterruptState nextState( _interruptState );
        bool changeState( false );
        if ( _interruptState == kDisabledNextCycle ) {
            nextState = kDisabled;
            changeState = true;
        }
        // If previous instruction was ei, disable interrupts after this instruction
        else if ( _interruptState == kEnabledNextCycle ) {
            nextState = kEnabled;
            changeState = true;
        }
        
        int nbCycles( 4 );
        if ( !_isHalted ) {
            nbCycles = execute( decodeOpcode() );
            if ( nbCycles < 0 ) {
                return nbCycles;
            }
        }

        if ( changeState ) {
            // Set the next interrupt state
            _interruptState = nextState;
        }
        
        return nbCycles;
    }

    const Registers& CPU::getRegisters() const
    {
        return *this;
    }
}
