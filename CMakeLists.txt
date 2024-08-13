cmake_minimum_required(VERSION 3.10)

project(izmirvm)

# Find Jitter executable
find_program(JITTER_EXECUTABLE jitter)
if(NOT JITTER_EXECUTABLE)
    message(FATAL_ERROR "Could not find Jitter executable. Please make sure it is installed and in your PATH.")
endif()

# Find Jitter library
find_library(JITTER_LIBRARY libjitter NAMES jitter PATHS ${JITTER_LIBRARY_DIR})
if(NOT JITTER_LIBRARY)
    message(FATAL_ERROR "Could not find Jitter library. Please make sure it is installed and in the specified directory.")
endif()

add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/izmirvm-vm.h ${CMAKE_SOURCE_DIR}/izmirvm-vm1.c ${CMAKE_SOURCE_DIR}/izmirvm-vm2.c ${CMAKE_SOURCE_DIR}/izmirvm-vm-main.c
    COMMAND ${JITTER_EXECUTABLE} --output ${CMAKE_SOURCE_DIR} --frontend ${CMAKE_SOURCE_DIR}/izmirvm.jitter
    DEPENDS ${CMAKE_SOURCE_DIR}/izmirvm.jitter
    VERBATIM
)

# Set Jitter include directories
set(JITTER_INCLUDE_DIRS
    # /usr/local/include/jitter/machine
    /usr/local/include
)

# Set Jitter library directory
set(JITTER_LIBRARY_DIR /usr/local/lib)

# Set Jitter libraries
set(JITTER_LIBRARIES ${JITTER_LIBRARY})

set(IZMIRVM_SOURCES
    izmirvm-vm.h
    izmirvm-vm1.c
    izmirvm-vm2.c
    izmirvm-vm-main.c
)

# Create the izmirvm executable
add_executable(izmirvm ${IZMIRVM_SOURCES})

# Link libraries
target_link_libraries(izmirvm ${JITTER_LIBRARIES})

# Compiler flags
target_compile_options(izmirvm PRIVATE
    -O2 -fomit-frame-pointer -fno-reorder-functions -fvisibility=hidden -fno-lto -g0 -fno-var-tracking -fno-var-tracking-assignments -fno-reorder-blocks -fno-reorder-blocks-and-partition -fno-crossjumping -fno-thread-jumps -fno-tree-tail-merge -fno-isolate-erroneous-paths-dereference -fno-split-paths -fPIC -fno-align-loops -fno-align-jumps -fno-align-labels -fno-jump-tables -fno-tree-switch-conversion -flive-range-shrinkage -fno-ipa-icf -fno-ipa-cp -fno-ipa-cp-clone -mcmodel=large
    -DJITTER_DISPATCH_NO_THREADING=1
)

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

# Generate scanner and parser files
add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/izmir-scanner.c ${CMAKE_SOURCE_DIR}/izmir-scanner.h
    COMMAND ${FLEX_EXECUTABLE} --header-file=${CMAKE_SOURCE_DIR}/izmir-scanner.h -o${CMAKE_SOURCE_DIR}/izmir-scanner.c ${CMAKE_SOURCE_DIR}/izmir.l
    DEPENDS ${CMAKE_SOURCE_DIR}/izmir.l
    VERBATIM
)
add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/izmir-parser.c ${CMAKE_SOURCE_DIR}/izmir-parser.h
    COMMAND ${BISON_EXECUTABLE} -o${CMAKE_SOURCE_DIR}/izmir-parser.c ${CMAKE_SOURCE_DIR}/izmir.y
    DEPENDS ${CMAKE_SOURCE_DIR}/izmir.y
    VERBATIM
)

set(IZMIR_SOURCES
    izmir-scanner.h
    izmir-parser.h
    izmir-scanner.c
    izmir-parser.c
    izmir-main.c
)

add_executable(izmir ${IZMIR_SOURCES})
target_link_libraries(izmir ${JITTER_LIBRARIES})

target_compile_options(izmir PRIVATE
    -O2 -fomit-frame-pointer -fno-reorder-functions -fvisibility=hidden -fno-lto -g0 -fno-var-tracking -fno-var-tracking-assignments -fno-reorder-blocks -fno-reorder-blocks-and-partition -fno-crossjumping -fno-thread-jumps -fno-tree-tail-merge -fno-isolate-erroneous-paths-dereference -fno-split-paths -fPIC -fno-align-loops -fno-align-jumps -fno-align-labels -fno-jump-tables -fno-tree-switch-conversion -flive-range-shrinkage -fno-ipa-icf -fno-ipa-cp -fno-ipa-cp-clone -mcmodel=large
    -DJITTER_DISPATCH_NO_THREADING=1
)
