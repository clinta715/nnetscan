cmake_minimum_required(VERSION 3.15)
project(NNetScan VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
    add_definitions(-DWINVER=0x0601)
endif()

add_executable(nnetscan
    src/main.cpp
    src/cli.cpp
    src/cli.h
    src/network.cpp
    src/network.h
    src/arp.cpp
    src/arp.h
    src/portscan.cpp
    src/portscan.h
    src/oui_lookup.cpp
    src/oui_lookup.h
    src/gui.cpp
    src/gui.h
)

target_link_libraries(nnetscan PRIVATE iphlpapi ws2_32)
target_compile_options(nnetscan PRIVATE /W4 /WX- /MT)
target_link_options(nnetscan PRIVATE /SUBSYSTEM:CONSOLE)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_link_options(nnetscan PRIVATE /LTCG /OPT:REF /OPT:ICF)
endif()
