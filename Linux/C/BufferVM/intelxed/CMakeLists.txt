cmake_minimum_required(VERSION 3.3)

SET(CMAKE_C_LINK_EXECUTABLE "ld")

project(intelxed C)

SET(INTELXED_LIB ${CMAKE_CURRENT_SOURCE_DIR}/kit/lib/libxed.a)

add_custom_command(OUTPUT ${INTELXED_LIB}
        COMMAND C_INCLUDE_PATH=../libc ${CMAKE_CURRENT_SOURCE_DIR}/xed/mfile.py install --install-dir ${CMAKE_CURRENT_SOURCE_DIR}/kit --extra-ccflags='-mcmodel=large -fno-stack-protector -mno-red-zone'
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_custom_target(intelxed
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS ${INTELXED_LIB})
