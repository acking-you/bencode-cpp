cmake_minimum_required(VERSION 3.21)
project(bencode)

set(CMAKE_CXX_STANDARD 20)
include_directories("../")
add_executable(test_bencode main.cpp "../bencode.cpp")