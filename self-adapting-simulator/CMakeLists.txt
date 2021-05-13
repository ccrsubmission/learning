project(self_adaptive_simulator)
cmake_minimum_required(VERSION 3.8)

find_package(Boost REQUIRED COMPONENTS system program_options filesystem python36 numpy36)
find_package(nlohmann_json REQUIRED)


find_package(PkgConfig REQUIRED)
pkg_check_modules(gsl sqlite3)
find_package(PythonLibs 3.6)


set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_CXX_FLAGS 
    "-std=c++17 -Wall -Wextra -Werror -Wsuggest-override ${CMAKE_CXX_FLAGS}")

set(CMAKE_CXX_FLAGS_RELEASE
    "-O2 -Wno-unused-parameter ${CMAKE_CXX_FLAGS_RELEASE}")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE ${CMAKE_EXE_LINKER_FLAGS_RELEASE})

add_subdirectory(src)
add_subdirectory(ns2/tools)
