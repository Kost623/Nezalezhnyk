cmake_minimum_required(VERSION 3.16)
project(Nezalezhnyk VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(nezalezhnyk
    src/main.cpp
    src/isa.cpp
    src/encoder.cpp
    src/alu.cpp
    src/memory.cpp
    src/cpu.cpp
)
target_include_directories(nezalezhnyk PRIVATE include)

add_executable(alu_tests
    tests/alu_tests.cpp
    src/isa.cpp
    src/alu.cpp
)
target_include_directories(alu_tests PRIVATE include)

add_executable(memory_tests
    tests/memory_tests.cpp
    src/isa.cpp
    src/memory.cpp
)
target_include_directories(memory_tests PRIVATE include)

add_executable(cpu_tests
    tests/cpu_tests.cpp
    src/isa.cpp
    src/alu.cpp
    src/memory.cpp
    src/cpu.cpp
    src/encoder.cpp
)
target_include_directories(cpu_tests PRIVATE include)
