//
//  glutapp.cpp
//  gbemu
//
//  Created by JF Boismenu on 2012-10-09.
//
//

// gbemu.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <iostream>
#include <cassert>
#include <iomanip>
#include <sstream>
#include <memory>
#include <fstream>
#include <thread>

#ifdef _WINDOWS
#include <Windows.h>
#endif
#ifdef _WINDOWS
#include <gl/glu.h>
#include <gl/glut.h>
#else
#include <GLUT/GLUT.h>
#endif
#include <set>

#include <common/common.h>
#include <memory/cartridgeInfo.h>
#include <memory/memory.h>
#include <cpu/cpu.h>
#include <video/videoDisplay.h>
#include <gameboy.h>
#include <gbemu.h>
#include <base/logger.h>
#include <base/audio.h>

namespace {

    using namespace gbemu;

    Gameboy* gbInstance;

    std::string getGameTitle( const Cartridge& cart )
    {
        static const unsigned short kMaxNameLength = 16;
        static const unsigned short kNameAddrStart = 0x134;
        static const unsigned short kNameAddrEnd = kMaxNameLength + kNameAddrStart;
        std::string result;
        result.reserve( kMaxNameLength );

        for ( unsigned short i = kNameAddrStart; cart.getByte( i ) != '\0' && i < kNameAddrEnd; ++i ) {
            result.push_back( (char)cart.getByte( i ) );
        }
        return result;
    }

    void printCartridgeInfo(
        const Cartridge& cart
    )
    {
        static const int kColumnWidth = 25;
        std::cout << std::setw( kColumnWidth ) << "Game Title : " << getGameTitle( cart ) << std::endl;
        std::cout << std::setw( kColumnWidth ) << "Cartridge Type : " << cartridgeInfo::cartridgeTypeToString( cart.getType() ) << std::endl;
        std::cout << std::setw( kColumnWidth ) << "ROM Size : " << cart.getROMSize() << " bytes" << std::endl;
        std::cout << std::setw( kColumnWidth ) << "RAM Size : " << cart.getRAMSize() << " bytes" << std::endl;
        std::cout << std::setw( kColumnWidth ) << "GameBoy Color Flag : " << ( cartridgeInfo::isGameBoyColor( cart ) ? "Yes" : "No" ) << std::endl;
        std::cout << std::setw( kColumnWidth ) << "Super GameBoy Flag : " << ( cartridgeInfo::isSuperGameBoy( cart ) ? "Yes" : "No" ) << std::endl;
    }

    void emulator()
    {
        if ( emulateSomeCycles( *gbInstance, 70224 ) ) {
            glutPostRedisplay();
        }
    }

    bool downPressed = false;
    bool upPressed = false;
    bool leftPressed = false;
    bool rightPressed = false;
    bool selectPressed = false;
    bool startPressed = false;
    bool aPressed = false;
    bool bPressed = false;

    void doButtons(
        const unsigned char key,
        const bool          setTo )
    {
        bool allButtonsPressed = toupper( key ) == 'R';
        bool selectStartB = toupper( key ) == 'M';
        if ( toupper( key ) == 'Z' || allButtonsPressed || selectStartB ) {
            bPressed = setTo;
        }
        if ( toupper( key ) == 'X' || allButtonsPressed ) {
            aPressed = setTo;
        }
        if ( toupper( key ) == 'A' || allButtonsPressed || selectStartB ) {
            selectPressed = setTo;
        }
        if ( toupper( key ) == 'S' || allButtonsPressed || selectStartB ) {
            startPressed = setTo;
        }
    }

    void doCross(
        const int  key,
        const bool setTo
    )
    {
        if ( key == GLUT_KEY_LEFT ) {
            leftPressed = setTo;
        }
        if ( key == GLUT_KEY_RIGHT ) {
            rightPressed = setTo;
        }
        if ( key == GLUT_KEY_UP ) {
            upPressed = setTo;
        }
        if ( key == GLUT_KEY_DOWN ) {
            downPressed = setTo;
        }
    }

    void writeStateToMemory()
    {
        const unsigned char keyState = GetMask(
            startPressed, // START
            selectPressed, // SELECT
            bPressed, // B
            aPressed, // A
            downPressed, // DOWN
            upPressed, // UP
            leftPressed, // LEFT
            rightPressed ); // RIGHT
        gbInstance->getMemory().setKeyState( keyState );
    }

    void keyboardDown(
        const unsigned char key,
        const int,
        const int
    )
    {
        doButtons( key, true );
        writeStateToMemory();
    }

    void keyboardUp(
        const unsigned char key,
        const int,
        const int
    )
    {
        doButtons( key, false );
        writeStateToMemory();
    }

    void specialDown(
        const int key,
        const int,
        const int
    )
    {
        doCross( key, true );
        writeStateToMemory();
    }

    void specialUp(
        const int key,
        const int,
        const int
    )
    {
        doCross( key, false );
        writeStateToMemory();
    }

    GLuint displayTexture;

    void calcFPS()
    {
        typedef std::chrono::high_resolution_clock CPUClock;
        typedef std::chrono::milliseconds milliseconds;

        CPUClock::time_point now = CPUClock::now();
        static CPUClock::time_point lastSecond = now;
        static int nbFramesSinceLastSecond = 0;
        ++nbFramesSinceLastSecond;

        milliseconds elapsed = std::chrono::duration_cast<milliseconds>(now - lastSecond);        
        if ( elapsed.count() > 1000 ) {
            std::cout << "FPS: " << float(nbFramesSinceLastSecond) / elapsed.count() * 1000 << std::endl;
            nbFramesSinceLastSecond = 0;
            lastSecond = now;
        }
    }

    void syncCpuWithAudio()
    {
        typedef std::chrono::high_resolution_clock CPUClock;
        typedef std::chrono::milliseconds milliseconds;

        const float audioLag = gbInstance->getClock().getTimeInSeconds() - gbInstance->getPAPU().getCurrentPlaybackTime();

        if (audioLag > 0.100) {
            //std::cout << "Audio lag: " << audioLag << std::endl;
            std::this_thread::sleep_for(milliseconds(int(audioLag * 1000 / 4)));
        }
    }

    void render(void)
    {
        // If we are perforrming slower than audio at the moment, do not sleep!
        if (gbInstance->getClock().getTimeInSeconds() < gbInstance->getPAPU().getCurrentPlaybackTime()) {
            return;
        }
        calcFPS();
        const Color* pixels = gbInstance->getVideo().getPixels();

        glBindTexture(GL_TEXTURE_2D, displayTexture);
        JFX_CMP_ASSERT( glGetError(), ==, GL_NO_ERROR );
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB,
            160, 144, 0,
            GL_RGB,
            GL_UNSIGNED_BYTE, pixels );
        JFX_CMP_ASSERT( glGetError(), ==, GL_NO_ERROR );
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST); // Linear Filtering
        JFX_CMP_ASSERT( glGetError(), ==, GL_NO_ERROR );
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST); // Linear Filtering
        JFX_CMP_ASSERT( glGetError(), ==, GL_NO_ERROR );
        glColor3f( 1, 1, 1 );
        glBegin( GL_QUADS );
        {
            glTexCoord2f( 0.f, 1.f );
            glVertex3f( -1.f, -1.f, 0.f );

            glTexCoord2f( 1.f, 1.f );
            glVertex3f( 1.f, -1.f, 0.f );

            glTexCoord2f( 1.f, 0.f );
            glVertex3f( 1.f, 1.f, 0.f );

            glTexCoord2f( 0.f, 0.f );
            glVertex3f( -1.f, 1.f, 0.f );
        }
        glEnd();

        glFlush();

        syncCpuWithAudio();
    }
}

int main(int argc, char* argv[])
{
    if ( argc == 1 ) {
        std::cout << "Missing cartridge name" << std::endl;
        return 0;
    }

    // Extract command line arguments
    const char* cartPath(0);
    const char* bootRomPath(0);
    // we support some -- arguments and two positional arguments.
    // -- arguments can be anywhere. Positional arguments are as follows:
    // 1) name of cartridge
    // 2) optional boot rom.
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--debug") {
            Logger::enableLogger(true);
        } else if (!cartPath) {
            cartPath = argv[i];
        } else if (!bootRomPath) {
            bootRomPath = argv[i];
        } else {
            std::cout << "Unexpected argument:" << argv[i] << std::endl;
            return -1;
        }
    }
    // Create the emulator.
    std::unique_ptr< Gameboy > gbInstanceGuard(
        gbemu::initGlobalEmulatorParams( cartPath, bootRomPath ) );
    gbInstance = gbInstanceGuard.get();

    Audio audio(
        44100, &gbInstance->getPAPU(), gbInstance->getPAPU().renderAudio
    );

    // Dump some info about the game we're about to play.
    printCartridgeInfo( gbInstance->getCartridge() );

    // Setup GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
    glutInitWindowPosition(1920/2,1200/2);
    glutInitWindowSize(160*3,144*3 );
    glutCreateWindow( "GBEmu" );
    glEnable( GL_TEXTURE_2D );
    glGenTextures( 1, &displayTexture );
    JFX_CMP_ASSERT( glGetError(), ==, GL_NO_ERROR );
    glutDisplayFunc( render );
    glutIdleFunc( emulator );
    glutKeyboardFunc( keyboardDown );
    glutKeyboardUpFunc( keyboardUp );
    glutSpecialFunc( specialDown );
    glutSpecialUpFunc( specialUp );
    audio.start();
    glutMainLoop();
    audio.stop();
	return 0;
}
