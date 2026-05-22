cmake_minimum_required(VERSION 3.20)
project(dos LANGUAGES NONE)

enable_language(CXX ASM_NASM)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(kernel)
