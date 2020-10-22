cmake_minimum_required(VERSION 3.3)

project(BufferVM)

include(../common/CMakeLists.txt)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror -Wno-unused-parameter" )

set(SOURCE_FILES main.c elf.c vm.c vm.h gdt.c paging.c ${COMMON_SOURCE_FILES} ../common/stdlib_inc.h vma.c vma.h ../common/host.h host.c host.h)
add_executable(buffervm ${SOURCE_FILES})

target_compile_definitions(buffervm PRIVATE VM)

add_dependencies(buffervm bootstrap)
target_link_libraries(buffervm bootstrap elf)
