cmake_minimum_required(VERSION 3.22)

project(bencode)

set(CMAKE_CXX_STANDARD 20)

#这里自己改库的安装路径
#SET(CMAKE_INSTALL_PREFIX "D:/lib/bencode/")

file(GLOB SRC_CXX src/*.cpp)
file(GLOB SRC_HXX src/*.h src/*.hpp)

add_library(bencode SHARED ${SRC_CXX}) #生成动态库

set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)
# 安装动态链接库
INSTALL(
        TARGETS  bencode
        # 安装LIBRARY 动态链接库
        LIBRARY
)

# 安装.h头文件
INSTALL(FILES ${SRC_HXX} DESTINATION include/bencode)

add_subdirectory(bencode_test)
