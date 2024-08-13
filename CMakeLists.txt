cmake_minimum_required(VERSION 3.20)

project(izmirvm)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

# --- Fetch jitter ---
FetchContent_Declare(
    jitter
    URL https://git.ageinghacker.net/jitter/snapshot/jitter-0.7.312.tar.xz
    URL_HASH SHA256=afe06782f4f4c7f18b9565fb4e52c618c22775f222a45747648d6f457c1c2bbb
)

FetchContent_GetProperties(jitter)

# --- Build jitter (custom autoconf pipeline) ---
if(NOT jitter_POPULATED)
    FetchContent_Populate(jitter)

    execute_process(
        COMMAND ${CMAKE_COMMAND} -E chdir ${jitter_SOURCE_DIR} ./bootstrap
        RESULT_VARIABLE BOOTSTRAP_RESULT
    )

    if(NOT BOOTSTRAP_RESULT EQUAL 0)
        message(FATAL_ERROR "Jitter bootstrap failed")
    endif()

    execute_process(
        COMMAND ${CMAKE_COMMAND} -E chdir ${jitter_SOURCE_DIR} ./configure --prefix=${jitter_BINARY_DIR}
        RESULT_VARIABLE CONFIGURE_RESULT
    )
    if(NOT CONFIGURE_RESULT EQUAL 0)
        message(FATAL_ERROR "Jitter configure failed")
    endif()

    execute_process(
        COMMAND ${CMAKE_COMMAND} -E chdir ${jitter_SOURCE_DIR} make
        RESULT_VARIABLE MAKE_RESULT
    )

    if(NOT MAKE_RESULT EQUAL 0)
        message(FATAL_ERROR "Jitter make failed")
    endif()

    execute_process(
        COMMAND ${CMAKE_COMMAND} -E chdir ${jitter_SOURCE_DIR} make install
        RESULT_VARIABLE MAKE_INSTALL_RESULT
    )

    if(NOT MAKE_INSTALL_RESULT EQUAL 0)
        message(FATAL_ERROR "Jitter make install failed")
    endif()

endif()


# --- Find all jitter related stuff ---
find_program(JITTER_EXECUTABLE jitter HINTS ${jitter_BINARY_DIR}/bin NO_DEFAULT_PATH)
find_library(JITTER_LIBRARY jitter PATHS ${jitter_BINARY_DIR}/lib NO_DEFAULT_PATH)
include_directories(${jitter_BINARY_DIR}/include)

if(NOT JITTER_EXECUTABLE OR NOT JITTER_LIBRARY)
    message(FATAL_ERROR "Could not find Jitter after installation. Please check the installation process.")
endif()

# message("JITTER_EXECUTABLE: ${JITTER_EXECUTABLE}")
# message("JITTER_LIBRARY: ${JITTER_LIBRARY}")

# --- Generate izmirvm-vm files ---
add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/izmirvm-vm.h ${CMAKE_SOURCE_DIR}/izmirvm-vm1.c ${CMAKE_SOURCE_DIR}/izmirvm-vm2.c ${CMAKE_SOURCE_DIR}/izmirvm-vm-main.c
    COMMAND ${JITTER_EXECUTABLE} --output ${CMAKE_SOURCE_DIR} --frontend ${CMAKE_SOURCE_DIR}/izmirvm.jitter
    DEPENDS ${CMAKE_SOURCE_DIR}/izmirvm.jitter
    VERBATIM
)

set(IZMIRVM_SOURCES
    izmirvm-vm.h
    izmirvm-vm1.c
    izmirvm-vm2.c
    izmirvm-vm-main.c
)

# --- Create the izmirvm executable ---
add_executable(izmirvm ${IZMIRVM_SOURCES})
target_link_libraries(izmirvm ${JITTER_LIBRARY})

# --- Compiler flags for izmirvm ---
target_compile_options(izmirvm PRIVATE
    -O2 -fomit-frame-pointer -fno-reorder-functions -fvisibility=hidden -fno-lto -g0 -fno-var-tracking -fno-var-tracking-assignments -fno-reorder-blocks -fno-reorder-blocks-and-partition -fno-crossjumping -fno-thread-jumps -fno-tree-tail-merge -fno-isolate-erroneous-paths-dereference -fno-split-paths -fPIC -fno-align-loops -fno-align-jumps -fno-align-labels -fno-jump-tables -fno-tree-switch-conversion -flive-range-shrinkage -fno-ipa-icf -fno-ipa-cp -fno-ipa-cp-clone -mcmodel=large
    -DJITTER_DISPATCH_NO_THREADING=1
)

# --- Flex and Bison Support ---
find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

# --- Generate scanner and parser files ---
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

# --- Create the izmir executable ---
add_executable(izmir ${IZMIR_SOURCES})
target_link_libraries(izmir ${JITTER_LIBRARY})

# --- Compiler flags for izmir ---
target_compile_options(izmir PRIVATE
    -O2 -fomit-frame-pointer -fno-reorder-functions -fvisibility=hidden -fno-lto -g0 -fno-var-tracking -fno-var-tracking-assignments -fno-reorder-blocks -fno-reorder-blocks-and-partition -fno-crossjumping -fno-thread-jumps -fno-tree-tail-merge -fno-isolate-erroneous-paths-dereference -fno-split-paths -fPIC -fno-align-loops -fno-align-jumps -fno-align-labels -fno-jump-tables -fno-tree-switch-conversion -flive-range-shrinkage -fno-ipa-icf -fno-ipa-cp -fno-ipa-cp-clone -mcmodel=large
    -DJITTER_DISPATCH_NO_THREADING=1
)
