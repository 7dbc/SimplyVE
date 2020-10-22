cmake_minimum_required(VERSION 3.3)

SET(CMAKE_C_LINK_EXECUTABLE "ld")

project(bootloader ASM)

set(SOURCE_FILES bootstrap.s)
add_library(bootstrap.o OBJECT ${SOURCE_FILES})

SET(BOOTSTRAP_S_OBJECT ${CMAKE_BINARY_DIR}/bootstrap/CMakeFiles/bootstrap.o.dir/bootstrap.s.o)
SET(BOOTSTRAP_OBJECT ${CMAKE_BINARY_DIR}/bootstrap/bootloader.o)

add_custom_command(OUTPUT ${BOOTSTRAP_OBJECT}
        COMMAND ${CMAKE_C_LINK_EXECUTABLE} -T ${CMAKE_SOURCE_DIR}/bootstrap/bootstrap.ld ${BOOTSTRAP_S_OBJECT} -o ${BOOTSTRAP_OBJECT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS bootstrap.o)

add_library(bootstrap
        STATIC
        ${BOOTSTRAP_OBJECT})

SET_TARGET_PROPERTIES(
        bootstrap
        PROPERTIES
        LINKER_LANGUAGE C
)