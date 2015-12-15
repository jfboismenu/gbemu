#pragma once

#include <cpu/opcode.h>
#include <cpu/registers.h>

namespace gbemu {

    class Cartridge;
    class Memory;

    class CPU : private Registers
    {
    public:

        CPU(
            Memory& memory,
            Cartridge& cartridge
        ) ;
        Opcode previewOpcode() const;
        int previewInstructionTiming() const;
        int emulateCycle();
        const Registers& getRegisters() const;
        bool areInterruptsEnabled() const;
        bool inBootRom() const;

        void executeInterrupt( unsigned short addr );

    private:
        void operator=( const CPU& cpu );

        enum InterruptState { kEnabled = 0, kDisabled = 1, kDisabledNextCycle = 2, kEnabledNextCycle = 3 };


        Opcode decodeOpcode();
        unsigned char readPCByte();
        char readPCSignedByte();
        unsigned short readPCWord();
        int execute( Opcode opcode );
        void updateLCD( int nbCycles );
        void handleInterrupts();

        void JP_nn() ;
        void XOR_n( const unsigned char value );
        void ld_r_n( unsigned char& reg );
        void ld_r_nn( unsigned short& reg );
        void ld_n_a( unsigned char& reg );
        void ld_nn_a( unsigned short addr );
        void ld_nn_sp();
        void ld_sp_hl();
        void ld_hl_sp_n();
        void ldd_hl_a();
        void ldd_a_hl();
        void ldi_hl_a();
        void ldi_a_hl();
        void ldh_n_a();
        void ldh_a_n();
        void ld_a_n( unsigned char value );
        void dec_n( unsigned char& value );
        void dec_nn( unsigned short& value );
        void jr_cc_n( bool flag );
        void jr_n();
        void jp_hl();
        void jp_cc_nn( bool flag );
        void rrc_n( unsigned char& reg, bool zeroMaskFlag = true );
        void rrc_mhl();
        void or_n( unsigned char value );
        void inc_n( unsigned char& value );
        void inc_nn( unsigned short& value );
        void dec_MemoryHL();
        void inc_MemoryHL();
        void ld_r1_r2(
            unsigned char& r1, unsigned char value
        );
        void ld_hl_r2( unsigned char value );
        void bit_b_r( unsigned int index, unsigned char value );
        void res_b_r( unsigned int index, unsigned char &value );
        void res_b_mhl( unsigned int index );
        void set_b_r( unsigned int index, unsigned char &value );
        void set_b_mhl( unsigned int index );
        void ld_ff00_c_a();
        void ld_a_ff00_c();
        void call_nn();
        void call_cc_nn( bool flag );
        void ret();
        void reti();
        void ret_cc( bool flag );
        void push_nn( unsigned short value );
        void pop_nn( unsigned short& value );
        void pop_af();
        void rr_n( unsigned char& value, bool zeroFlagMask = true );
        void rr_hl();
        void rl_n( unsigned char& value, bool zeroFlagMask = true );
        void rl_mhl();
        void rlc_n( unsigned char& value, bool zeroFlagMask = true );
        void rlca();
        void rlc_mhl();
        void srl_n( unsigned char& reg );
        void srl_mhl();
        void sla_n( unsigned char& value );
        void sla_mhl();
        void sra_n( unsigned char& value );
        void sra_mhl();
        void cp_n( unsigned char value );
        void and_n( unsigned char value );
        void cpl();
        void sub_n( unsigned char value );
        void add_n( unsigned char value );
		void add_sp_n();
        void adc_a_n( unsigned char value );
        void sbc_a_n( unsigned char value );
        void add_hl_n( unsigned short value );
        void di();
        void ei();
        void halt();
        void swap_mhl();
        void swap_n( unsigned char& reg );
        void rst( unsigned short offset );
        void daa();
        void scf();
        void ccf();

        unsigned char subImp( unsigned char left, unsigned char right, bool carry = false );
        unsigned char addImp( unsigned char left, unsigned char right, bool carry = false );
        unsigned short addWordImp( unsigned short left, unsigned short right );

        Memory& _memory;
        Cartridge& _cartridge;

        bool _inBootROM;
        unsigned char _bootROM[ 256 ];
        InterruptState _interruptState;

        unsigned char _opTime[ 256 ];
        unsigned char _opTimeCb[ 256 ];
        bool _isHalted;
    };
}
