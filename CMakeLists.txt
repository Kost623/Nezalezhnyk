cmake_minimum_required(VERSION 3.16)
project(Nezalezhnyk VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(nzk_core STATIC
    src/isa.cpp
    src/encoder.cpp
    src/alu.cpp
    src/memory.cpp
    src/cpu.cpp
)
target_include_directories(nzk_core PUBLIC include)

add_executable(nezalezhnyk src/main.cpp)
target_link_libraries(nezalezhnyk PRIVATE nzk_core)

add_executable(alu_tests tests/alu_tests.cpp)
target_link_libraries(alu_tests PRIVATE nzk_core)

add_executable(memory_tests tests/memory_tests.cpp)
target_link_libraries(memory_tests PRIVATE nzk_core)

add_executable(cpu_tests tests/cpu_tests.cpp)
target_link_libraries(cpu_tests PRIVATE nzk_core)

# --- GUI (опційно, потребує Qt6) ---
option(NZK_BUILD_GUI "Збирати Qt GUI" ON)
if(NZK_BUILD_GUI)
    find_package(Qt6 COMPONENTS Widgets QUIET)
    if(Qt6_FOUND)
        set(CMAKE_AUTOMOC ON)
        add_executable(nezalezhnyk_gui
            src/gui/main.cpp
            src/gui/mainwindow.cpp
            src/gui/mainwindow.hpp
            src/gui/register_model.hpp
        )
        target_link_libraries(nezalezhnyk_gui PRIVATE nzk_core Qt6::Widgets)
    else()
        message(STATUS "Qt6 не знайдено — ціль nezalezhnyk_gui пропущена (sudo pacman -S qt6-base)")
    endif()
endif()