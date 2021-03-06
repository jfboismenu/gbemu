cmake_minimum_required(VERSION 2.6)
project(gbemu)
find_package(OpenGL REQUIRED)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

# Glut is not standard on Windows, so use the copy provided in the repo.
if (WIN32)
	include_directories( "thirdparties/glut/includes" )
	set(GLUT_LIBRARY "thirdparties/glut/lib/freeglut_static")
	add_definitions(-DFREEGLUT_STATIC)
else (WIN32)
	find_package(GLUT REQUIRED)
endif (WIN32)

include_directories( ${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS} . /usr/local/include)

include_directories()
set_source_files_properties(../audio.cpp PROPERTIES COMPILE_FLAGS
    "-Wno-deprecated-declarations")
link_directories(/usr/local/lib)

add_library(
    gbemulib
    base/logger.cpp base/clock.cpp base/counter.cpp
    common/register.cpp common/common.cpp
    cpu/cpu.cpp cpu/opcode.cpp cpu/timers.cpp cpu/registers.cpp
    video/videoDisplay.cpp
    memory/bootRom.cpp memory/mbc.cpp memory/memory.cpp memory/cartridgeInfo.cpp memory/memoryRegion.cpp
    audio/common.cpp audio/channelBase.cpp audio/papu.cpp audio/squareWaveChannel.cpp audio/waveChannel.cpp audio/envelope.cpp audio/frequency.cpp
    gameboy.cpp gbemu.cpp
)

add_executable(
    tests
    tests/testMain.cpp
)

add_executable(
    gbemu
    base/audio.cpp glutapp.cpp
)

target_link_libraries(tests gbemulib ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} portaudio)

target_link_libraries(gbemu gbemulib ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} portaudio)
