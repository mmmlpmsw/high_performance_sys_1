cmake_minimum_required(VERSION 3.16)

project(lab1 C)

set(CMAKE_C_COMPILER "/usr/local/opt/llvm/bin/clang")
set(CMAKE_C_STANDARD 99)

set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-L/usr/local/opt/llvm/lib -fsanitize=address -fsanitize=undefined -Wall -pedantic -fopenmp")

add_executable(lab1 main.c selection_sort.c selection_sort.h tim_sort.c tim_sort.h util.h util.c)