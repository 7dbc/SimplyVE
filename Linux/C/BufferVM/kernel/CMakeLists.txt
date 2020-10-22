cmake_minimum_required(VERSION 3.3)

project(kernel C ASM)

include(../common/CMakeLists.txt)

include_directories(../libc)

set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static -no-pie -Wall -Wextra -Werror -Wno-unused-parameter -Wno-switch -Wno-unused-variable -Wno-return-type -Wno-old-style-declaration -Wno-override-init" )
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffreestanding -fPIC -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -nostdlib -lgcc -fdata-sections -ffunction-sections" )

set(CMAKE_EXE_LINKER_FLAGS "-Wl,--gc-sections,--build-id=none,--no-dynamic-linker" )

set(SOURCE_FILES asm/start.S c/kernel.c asm/host.S c/idt.c c/gdt.c asm/idt.S asm/page_fault.S c/cstubs.c asm/tss.S c/tss.c c/syscall.c asm/syscall.S asm/cpu.S c/version.c ${COMMON_SOURCE_FILES} c/mmap.c c/stack.c c/page_fault.c c/utils.c ../common/utils.h c/vma_heap.c c/elf.c c/cpu.c c/rbtree.c c/vma.c)

SET(CMAKE_C_LINK_EXECUTABLE "${PROJECT_SOURCE_DIR}/linker.py -c '${CMAKE_C_FLAGS}' -l '<LINK_FLAGS>' -L '<LINK_LIBRARIES>' -s ${PROJECT_SOURCE_DIR}/kernel.ld -o '<OBJECTS>' -t <TARGET>")

add_executable(kernel ${SOURCE_FILES})

target_compile_definitions(kernel PRIVATE KERNEL NOSTDLIB_BUILD)

add_dependencies(kernel intelxed)
target_link_libraries(kernel c ${CMAKE_SOURCE_DIR}/intelxed/kit/lib/libxed.a)

set_target_properties(kernel PROPERTIES SUFFIX ".elf")
