include_directories(../src)
include_directories(../lib)

set(SRC_CXX
        main.cpp
        )
file(GLOB BSRC ${CMAKE_SOURCE_DIR}/src/*.cpp)

add_definitions(-DU_DICT)

include_directories(${CMAKE_SOURCE_DIR}/src)

add_executable(test_bencode ${SRC_CXX} ${BSRC})
