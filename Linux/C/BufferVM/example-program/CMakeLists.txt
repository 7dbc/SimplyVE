cmake_minimum_required(VERSION 3.3)

project(ExampleProgram)

enable_language(ASM)

include_directories(../libc)

set(CMAKE_C_FLAGS "-g -w -m64 -nostdlib -fno-stack-protector")
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror" )

set(CMAKE_EXE_LINKER_FLAGS "-Wl,--no-dynamic-linker" )

set(SOURCE_FILES sbrk.c alloc.c main.c start.s cstubs.S)
add_executable(example-program ${SOURCE_FILES})

target_link_libraries(example-program c)
