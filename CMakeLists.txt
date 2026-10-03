# feito por: maquinzz
cmake_minimum_required(VERSION 3.16)
project(file-integrity-monitor VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_executable(fim src/main.cpp src/sha256.cpp src/fim.cpp)
target_include_directories(fim PRIVATE include)
target_compile_options(fim PRIVATE -Wall -Wextra -Wpedantic -Werror -D_FORTIFY_SOURCE=2 -fstack-protector-strong)
