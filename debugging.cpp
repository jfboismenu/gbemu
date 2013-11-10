#include "debugging.h"
#include "memory.h"
#include "registers.h"

#include <sstream>

#if 0
namespace {
    using namespace gbemu;

    const std::string kByteRegisterNames[] = { "a", "b", "c", "d", "e", "f", "h", "l", "(hl)", "(bc)", "(de)", "(nn)", "#" };
    const std::string kWordRegisterNames[] = { "af", "bc", "de", "hl", "pc", "sp", "(hl)" };
    const std::string kFlagNames[] = { "z", "c", "h", "nz", "nc", "nh" };
    enum FlagIndex { 
        kZ = 0,
        kC = 1,
        kH = 2,
        kNZ = 3,
        kNC = 4,
        kNH = 5 };

    int readSignedByte( const unsigned char* bytes )
    {
        return (int)( static_cast< char >( *bytes ) );
    }

    unsigned short readWord( const unsigned char* bytes )
    {
        return *reinterpret_cast< const unsigned short* >( bytes );
    }

    unsigned int readByte( const unsigned char* bytes )
    {
        return (unsigned int)( *bytes );
    }

    void nopToString( std::ostream& os, const unsigned char* )
    {
        os << "nop";
    }

    void diToString( std::ostream& os, const unsigned char* )
    {
        os << "di";
    }

    void eiToString( std::ostream& os, const unsigned char* )
    {
        os << "ei";
    }

    void jpToString( std::ostream& os, const unsigned char* bytes )
    {
        os << "jp " << "0x" << std::hex << readWord( bytes + 1 );
    }

    void jpmhlToString( std::ostream& os, const unsigned char* )
    {
        os << "jp (hl)";
    }

    template< FlagIndex Index >
    void jp_cc_nToString( std::ostream& os, const unsigned char* bytes )
    {
        os << "jp " << kFlagNames[ Index ] << ", " << readWord( bytes + 1 );
    }

    template< ByteRegister RegisterIndex >
    void xorByteToString( std::ostream& os, const unsigned char* )
    {
        os << "xor a, " << kByteRegisterNames[ RegisterIndex ];
    }

    template<>
    void xorByteToString< kMPC >( std::ostream& os, const unsigned char* bytes )
    {
        os << "xor a, " << readByte( bytes + 1 );
    }

    template< ByteRegister Index >
    void ld_n_nToString( std::ostream& os, const unsigned char* bytes )
    {
        os << "ld " << kByteRegisterNames[ Index ] << ", " << std::hex << readByte( bytes + 1 );
    }

    template< ByteRegister Index >
    void ld_n_aToString( std::ostream& os, const unsigned char* )
    {
        os << "ld " << kByteRegisterNames[ Index ] << ", a";
    }

    template< ByteRegister Index >
    void cp_nToString( std::ostream& os, const unsigned char* )
    {
        os << "cp " << kByteRegisterNames[ Index ];
    }

    template<>
    void cp_nToString< kMPC >( std::ostream& os, const unsigned char* bytes )
    {
        os << "cp " << readByte( bytes + 1 );
    }

    template< WordRegister Index >
    void ld_a_nnToString( std::ostream& os, const unsigned char* )
    {
        os << "ld a, (" << kWordRegisterNames[ Index ] << ")";
    }

    template<>
    void ld_a_nnToString< kMWPC >( std::ostream& os, const unsigned char* bytes )
    {
        os << "ld a, " << std::hex << readByte( bytes + 1 );
    }

    void ldd_hl_aToString( std::ostream& os, const unsigned char* )
    {
        os << "ldd (hl), a";
    }

    void ldi_hl_aToString( std::ostream& os, const unsigned char* )
    {
        os << "ldi (hl), a";
    }

    void ldd_a_hlToString( std::ostream& os, const unsigned char* )
    {
        os << "ldd a, (hl)";
    }

    template< ByteRegister Dst, ByteRegister Src >
    void ld_r1_r2ToString( std::ostream& os, const unsigned char* )
    {
        os << "ld " << kByteRegisterNames[ Dst ] << ", " << kByteRegisterNames[ Src ];
    }

    void ld_mhl_n( std::ostream& os, const unsigned char* bytes )
    {
        os << "ld (hl), " << std::hex << readByte( bytes + 1 );
    }

    template< ByteRegister Dst >
    void registerLD_r1_ByteRegisterHandlers( Opcode base, DebugStringHandlerRegistry& reg )
    {
        reg.registerHandler( Opcode( base + 0 ), &ld_r1_r2ToString< Dst, kB > );
        reg.registerHandler( Opcode( base + 1 ), &ld_r1_r2ToString< Dst, kC > );
        reg.registerHandler( Opcode( base + 2 ), &ld_r1_r2ToString< Dst, kD > );
        reg.registerHandler( Opcode( base + 3 ), &ld_r1_r2ToString< Dst, kE > );
        reg.registerHandler( Opcode( base + 4 ), &ld_r1_r2ToString< Dst, kH > );
        reg.registerHandler( Opcode( base + 5 ), &ld_r1_r2ToString< Dst, kL > );
    }

    template< ByteRegister Dst >
    void registerLD_r1_r2Handlers( Opcode base, DebugStringHandlerRegistry& reg )
    {
        registerLD_r1_ByteRegisterHandlers< Dst >( base, reg );
        reg.registerHandler( Opcode( base + 6 ), &ld_r1_r2ToString< Dst, kMHL > );
    }

    template<>
    void registerLD_r1_r2Handlers< kMHL >( Opcode base, DebugStringHandlerRegistry& reg )
    {
        registerLD_r1_ByteRegisterHandlers< kMHL >( base, reg );
        reg.registerHandler( LD_HL_n, &ld_mhl_n );
    }

    void ldi_a_hlToString( std::ostream& os, const unsigned char* )
    {
        os << "ldi a, (hl)";
    }

    template< WordRegister RegisterIndex >
    void load16BitToString( std::ostream& os, const unsigned char* bytes )
    {
        os << "ld " << kWordRegisterNames[ RegisterIndex ] << ", 0x" << std::hex << readWord( bytes + 1 );
    }

    template< ByteRegister Index >
    void dec_nToString( std::ostream& os, const unsigned char* )
    {
        os << "dec " << kByteRegisterNames[ Index ];
    }

    template< WordRegister Index >
    void dec_nnToString( std::ostream& os, const unsigned char* )
    {
        os << "dec " << kWordRegisterNames[ Index ];
    }

    template< FlagIndex Index >
    void jr_cc_nToString( std::ostream& os, const unsigned char* bytes )
    {
        os << "jr " << kFlagNames[ Index ] << " " << std::dec << "dec(" << readSignedByte( bytes + 1 ) << ")";
    }

    void jr_nToString( std::ostream& os, const unsigned char* bytes )
    {
        os << "jr dec(" << std::dec << readSignedByte( bytes + 1 ) << ")";
    }

    void rraToString( std::ostream& os, const unsigned char* )
    {
        os << "rra";
    }

    template< ByteRegister Index >
    void or_nToString( std::ostream& os, const unsigned char* )
    {
        os << "or " << kByteRegisterNames[ Index ];
    }

    template< ByteRegister Index >
    void inc_nToString( std::ostream& os, const unsigned char* )
    {
        os << "inc " << kByteRegisterNames[ Index ];
    }

    template< WordRegister Index >
    void inc_nnToString( std::ostream& os, const unsigned char* )
    {
        os << "inc " << kWordRegisterNames[ Index ];
    }

    template< int BitIndex, ByteRegister Index >
    void bit_b_r( std::ostream& os, const unsigned char* )
    {
        os << "bit " << BitIndex << ", " << kByteRegisterNames[ Index ];
    }

    void ldh_ff00_n_AToString( std::ostream& os, const unsigned char* bytes )
    {
        os << "ld $(ff00+" << std::hex << readByte( bytes + 1 ) << "), a";
    }

    void ldh_A_ff00_nToString( std::ostream& os, const unsigned char* bytes )
    {
        os << "ld a, $(ff00+" << std::hex << readByte( bytes + 1 ) << ")";
    }

    void ldh_ff00_C_AToString( std::ostream& os, const unsigned char* )
    {
        os << "ld $(ff00+c), a";
    }

    void ldh_A_ff00_CToString( std::ostream& os, const unsigned char* )
    {
        os << "ld a, $(ff00+c)";
    }

    template< FlagIndex Index >
    void call_cc_nnToString( std::ostream& os, const unsigned char* bytes )
    {
        os << "call " << kFlagNames[ Index ] << ", " << std::hex << readWord( bytes + 1 ) << std::endl;
    }

    void call_nnToString( std::ostream& os, const unsigned char* bytes )
    {
        os << "call " << std::hex << readWord( bytes + 1 ) << std::endl;
    }
}
#endif
namespace gbemu
{    
    DebugStringHandlerRegistry::DebugStringHandlerRegistry()
    {
        for ( int i = 0; i < 256; ++i ) {
            _handlers[ i ] = 0;
            _cbHandlers[ i ] = 0;
        }
    }
    
    void DebugStringHandlerRegistry::registerHandler(
        Opcode opcode,
        OpcodeStringConverter handler
    )
    {
        if ( isCBOpcode( opcode ) ) {
            JFX_ASSERT( 0 == _cbHandlers[ getDifferentiatingOpcodeByte( opcode ) ] );
            _cbHandlers[ getDifferentiatingOpcodeByte( opcode ) ] = handler;
        }
        else {
            JFX_ASSERT( 0 == _handlers[ opcode ] );
            _handlers[ opcode ] = handler;
        }
    }

    OpcodeStringConverter DebugStringHandlerRegistry::getHandler( const Opcode opcode ) const
    {
        if ( isCBOpcode( opcode ) ) {
            return _cbHandlers[ getDifferentiatingOpcodeByte( opcode ) ];
        }
        else {
            return _handlers[ opcode ];
        }
    }

    void initializeDebugStringHandlers( DebugStringHandlerRegistry& /*reg*/ )
    {
#if 0
        reg.registerHandler( kNop, &nopToString );
        reg.registerHandler( EI, &eiToString );
        reg.registerHandler( DI, &diToString );
        reg.registerHandler( LDH_n_A, &ldh_ff00_n_AToString );
        reg.registerHandler( LDH_A_n, &ldh_A_ff00_nToString );
        reg.registerHandler( LD_FF00_C_A, &ldh_ff00_C_AToString );
        reg.registerHandler( LD_A_FF00_C, &ldh_A_ff00_CToString );

        // jumps
        reg.registerHandler( kJP, &jpToString );
        reg.registerHandler( JP_MHL, &jpmhlToString );
        reg.registerHandler( JP_NZ_nn, &jp_cc_nToString< FlagIndex::kNZ > );
        reg.registerHandler( JP_Z_nn,  &jp_cc_nToString< FlagIndex::kZ > );
        reg.registerHandler( JP_NC_nn, &jp_cc_nToString< FlagIndex::kNC > );
        reg.registerHandler( JP_C_nn,  &jp_cc_nToString< FlagIndex::kC > );

        // 8-bit loads

        reg.registerHandler( LD_A_n, &ld_n_nToString< kA > );
        reg.registerHandler( LD_B_n, &ld_n_nToString< kB > );
        reg.registerHandler( LD_C_n, &ld_n_nToString< kC > );
        reg.registerHandler( LD_D_n, &ld_n_nToString< kD > );
        reg.registerHandler( LD_E_n, &ld_n_nToString< kE > );
        reg.registerHandler( LD_H_n, &ld_n_nToString< kH > );
        reg.registerHandler( LD_L_n, &ld_n_nToString< kL > );

        reg.registerHandler( LD_A_BC, &ld_a_nnToString< kBC > );
        reg.registerHandler( LD_A_DE, &ld_a_nnToString< kDE > );
        reg.registerHandler( LD_A_HL, &ld_a_nnToString< kHL > );
        reg.registerHandler( LD_A_nn, &ld_a_nnToString< kMWPC > );

        reg.registerHandler( LDD_HL_A, &ldd_hl_aToString );
        reg.registerHandler( LDI_HL_A, &ldi_hl_aToString );
        reg.registerHandler( LDI_A_HL, &ldi_a_hlToString );
        reg.registerHandler( LDD_A_HL, &ldd_a_hlToString );

        registerLD_r1_r2Handlers< kB >( LD_B_B, reg );
        registerLD_r1_r2Handlers< kC >( LD_C_B, reg );
        registerLD_r1_r2Handlers< kD >( LD_D_B, reg );
        registerLD_r1_r2Handlers< kE >( LD_E_B, reg );
        registerLD_r1_r2Handlers< kH >( LD_H_B, reg );
        registerLD_r1_r2Handlers< kL >( LD_L_B, reg );
        registerLD_r1_r2Handlers< kMHL >( LD_HL_B, reg );

        // XORs
        reg.registerHandler( kXOR_A, &xorByteToString< kA > );
        reg.registerHandler( kXOR_B, &xorByteToString< kB > );
        reg.registerHandler( kXOR_C, &xorByteToString< kC > );
        reg.registerHandler( kXOR_D, &xorByteToString< kD > );
        reg.registerHandler( kXOR_E, &xorByteToString< kE > );
        reg.registerHandler( kXOR_H, &xorByteToString< kH > );
        reg.registerHandler( kXOR_L, &xorByteToString< kL > );
        reg.registerHandler( kXOR_HL, &xorByteToString< kMHL > );
        reg.registerHandler( kXOR_STAR, &xorByteToString< kMPC > );

        // 16-bit loads
        reg.registerHandler( LD_BC_nn,     &load16BitToString< kBC > );
        reg.registerHandler( LD_DE_nn,     &load16BitToString< kDE > );
        reg.registerHandler( LD_HL_nn,     &load16BitToString< kHL > );
        reg.registerHandler( LD_SP_nn,     &load16BitToString< kSP > );

        // 8-bit loads

        reg.registerHandler( DEC_A,      &dec_nToString< kA > );
        reg.registerHandler( DEC_B,      &dec_nToString< kB > );
        reg.registerHandler( DEC_C,      &dec_nToString< kC > );
        reg.registerHandler( DEC_D,      &dec_nToString< kD > );
        reg.registerHandler( DEC_E,      &dec_nToString< kE > );
        reg.registerHandler( DEC_H,      &dec_nToString< kH > );
        reg.registerHandler( DEC_L,      &dec_nToString< kL > );
        reg.registerHandler( DEC_MHL,     &dec_nToString< kMHL > );

        reg.registerHandler( DEC_BC, &dec_nnToString< kBC > );
        reg.registerHandler( DEC_DE, &dec_nnToString< kDE > );
        reg.registerHandler( DEC_HL, &dec_nnToString< kHL > );
        reg.registerHandler( DEC_SP, &dec_nnToString< kSP > );

        reg.registerHandler( JR_NZ_n,    &jr_cc_nToString< FlagIndex::kNZ > );
        reg.registerHandler( JR_Z_n,     &jr_cc_nToString< FlagIndex::kZ > );
        reg.registerHandler( JR_NC_n,    &jr_cc_nToString< FlagIndex::kNC > );
        reg.registerHandler( JR_C_n,     &jr_cc_nToString< FlagIndex::kC > );

        reg.registerHandler( JR_n,     &jr_nToString );

        reg.registerHandler( RRA,        &rraToString );

        reg.registerHandler( OR_A,       &or_nToString< kA > );
        reg.registerHandler( OR_B,       &or_nToString< kB > );
        reg.registerHandler( OR_C,       &or_nToString< kC > );
        reg.registerHandler( OR_D,       &or_nToString< kD > );
        reg.registerHandler( OR_E,       &or_nToString< kE > );
        reg.registerHandler( OR_H,       &or_nToString< kH > );
        reg.registerHandler( OR_L,       &or_nToString< kL > );
        reg.registerHandler( OR_HL,      &or_nToString< kMHL > );
        reg.registerHandler( OR_STAR,    &or_nToString< kMPC > );

        reg.registerHandler( INC_A,       &inc_nToString< kA > );
        reg.registerHandler( INC_B,       &inc_nToString< kB > );
        reg.registerHandler( INC_C,       &inc_nToString< kC > );
        reg.registerHandler( INC_D,       &inc_nToString< kD > );
        reg.registerHandler( INC_E,       &inc_nToString< kE > );
        reg.registerHandler( INC_H,       &inc_nToString< kH > );
        reg.registerHandler( INC_L,       &inc_nToString< kL > );
        reg.registerHandler( INC_MHL,      &inc_nToString< kMHL > );

        reg.registerHandler( INC_BC,  &inc_nnToString< kBC > );
        reg.registerHandler( INC_DE,  &inc_nnToString< kDE > );
        reg.registerHandler( INC_HL,  &inc_nnToString< kHL > );
        reg.registerHandler( INC_SP,  &inc_nnToString< kSP > );

        reg.registerHandler( BIT_7_H,      &bit_b_r< 7, kH > );

        reg.registerHandler( LD_B_A , &ld_n_aToString< kB   > );
        reg.registerHandler( LD_C_A , &ld_n_aToString< kC   > );
        reg.registerHandler( LD_D_A , &ld_n_aToString< kD   > );
        reg.registerHandler( LD_E_A , &ld_n_aToString< kE   > );
        reg.registerHandler( LD_H_A , &ld_n_aToString< kH   > );
        reg.registerHandler( LD_L_A , &ld_n_aToString< kL   > );
        reg.registerHandler( LD_BC_A, &ld_n_aToString< kMBC > );
        reg.registerHandler( LD_DE_A, &ld_n_aToString< kMDE > );
        reg.registerHandler( LD_HL_A, &ld_n_aToString< kMHL > );
        reg.registerHandler( LD_nn_A, &ld_n_aToString< kMNN > );

        reg.registerHandler( CP_A,  &cp_nToString< kA > ); 
        reg.registerHandler( CP_B,  &cp_nToString< kB > ); 
        reg.registerHandler( CP_C,  &cp_nToString< kC > ); 
        reg.registerHandler( CP_D,  &cp_nToString< kD > ); 
        reg.registerHandler( CP_E,  &cp_nToString< kE > ); 
        reg.registerHandler( CP_H,  &cp_nToString< kH > ); 
        reg.registerHandler( CP_L,  &cp_nToString< kL > ); 
        reg.registerHandler( CP_HL, &cp_nToString< kMHL > );
        reg.registerHandler( CP_N,  &cp_nToString< kMPC > );
        
        reg.registerHandler( CALL_NZ_nn, &call_cc_nnToString< FlagIndex::kNZ > );
        reg.registerHandler( CALL_Z_nn,  &call_cc_nnToString< FlagIndex::kZ > );
        reg.registerHandler( CALL_NC_nn, &call_cc_nnToString< FlagIndex::kNC > ); 
        reg.registerHandler( CALL_C_nn,  &call_cc_nnToString< FlagIndex::kC > );

        reg.registerHandler( CALL_nn, &call_nnToString );
#endif
    }
}
