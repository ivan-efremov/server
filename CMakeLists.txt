cmake_minimum_required(VERSION 3.2.2)
project(server)

set(CMAKE_CXX_STANDARD 17)


set(CMAKE_CXX_FLAGS_DEBUG "-pipe -DDEBUG -Wall -gdwarf-3 -gstrict-dwarf -O0 ${CMAKE_CXX_FLAGS_ALL}")

set(CMAKE_CXX_FLAGS_RELEASE "-pipe -DNDEBUG -Wall -g0 -O2 -s ${CMAKE_CXX_FLAGS_ALL}")

set(SOURCE_FILES TcpServer.cpp main.cpp)

add_executable(server ${SOURCE_FILES})

target_link_libraries(server boost_system boost_program_options pthread)
