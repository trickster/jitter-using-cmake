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

FetchContent_Populate(jitter)

# this will install jitter tool, for shared library
# execute_process(
#     COMMAND ${CMAKE_COMMAND} -E chdir ${jitter_SOURCE_DIR} ./bootstrap
#     RESULT_VARIABLE BOOTSTRAP_RESULT
# )

# if(NOT BOOTSTRAP_RESULT EQUAL 0)
#     message(FATAL_ERROR "Jitter bootstrap failed")
# endif()

# execute_process(
#     COMMAND ${CMAKE_COMMAND} -E chdir ${jitter_SOURCE_DIR} ./configure --prefix=${jitter_BINARY_DIR}
#     RESULT_VARIABLE CONFIGURE_RESULT
# )
# if(NOT CONFIGURE_RESULT EQUAL 0)
#     message(FATAL_ERROR "Jitter configure failed")
# endif()

# execute_process(
#     COMMAND ${CMAKE_COMMAND} -E chdir ${jitter_SOURCE_DIR} make
#     RESULT_VARIABLE MAKE_RESULT
# )

# if(NOT MAKE_RESULT EQUAL 0)
#     message(FATAL_ERROR "Jitter make failed")
# endif()

# execute_process(
#     COMMAND ${CMAKE_COMMAND} -E chdir ${jitter_SOURCE_DIR} make install
#     RESULT_VARIABLE MAKE_INSTALL_RESULT
# )

# if(NOT MAKE_INSTALL_RESULT EQUAL 0)
#     message(FATAL_ERROR "Jitter make install failed")
# endif()

# do this again to make shared library, so annoying

# execute_process(
#     COMMAND ${CMAKE_COMMAND} -E chdir ${jitter_SOURCE_DIR} make clean
#     RESULT_VARIABLE MAKE_RESULT_CLEAN
# )

# if(NOT MAKE_RESULT_CLEAN EQUAL 0)
#     message(FATAL_ERROR "make clean failed")
# endif()

set(ENV{JITTER_SUBPACKAGE} "1")
set(ENV{CFLAGS} "-g -O2 -fPIC -fno-omit-frame-pointer")
set(ENV{LDFLAGS} "-Wl,--export-dynamic")

execute_process(
    COMMAND ${CMAKE_COMMAND} -E env
        JITTER_SUBPACKAGE=1
        # CFLAGS=-g -O2 -fPIC -fno-omit-frame-pointer
        # LDFLAGS=-Wl,--export-dynamic
        ${CMAKE_COMMAND} -E chdir ${jitter_SOURCE_DIR} ./configure
    RESULT_VARIABLE CONFIGURE_RESULT
)
if(NOT CONFIGURE_RESULT EQUAL 0)
    message(FATAL_ERROR "Jitter configure failed (shared)")
endif()

execute_process(
    COMMAND ${CMAKE_COMMAND} -E chdir ${jitter_SOURCE_DIR} rm -rf lib non-convenience-lib
    RESULT_VARIABLE DELETE_STUFF
)

if(NOT DELETE_STUFF EQUAL 0)
    message(FATAL_ERROR "deleting failed")
endif()

execute_process(
    COMMAND ${CMAKE_COMMAND} -E chdir ${jitter_SOURCE_DIR} make clean
    RESULT_VARIABLE MAKE_RESULT
)

if(NOT MAKE_RESULT EQUAL 0)
    message(FATAL_ERROR "make clean failed (shared)")
endif()

execute_process(
    COMMAND ${CMAKE_COMMAND} -E chdir ${jitter_SOURCE_DIR} make
    RESULT_VARIABLE MAKE_RESULT
)

if(NOT MAKE_RESULT EQUAL 0)
    message(FATAL_ERROR "Jitter make failed (shared)")
endif()

# --- Find all jitter related stuff ---
# JITTER_SUBPACKAGE_DIRECTORY=1 ./configure
# set(JITTER_LIBRARY /home/user/jitter-shared/libjitter.a)
# set(JITTER_LIBRARY /home/user/izmirstuff/izmir-2.0/build/_deps/jitter-src/non-convenience-lib/.libs/libjitter.so)
# set(JITTER_LIBRARY /home/user/izmirstuff/izmir-2.0/build/_deps/jitter-src/lib/libjitter.a)

find_program(JITTER_EXECUTABLE jitter HINTS ${jitter_BINARY_DIR}/bin NO_DEFAULT_PATH)
# this is important here, make install doesn't work, so SOURCE_DIR has all the shared so
find_library(JITTER_LIBRARY jitter PATHS ${jitter_SOURCE_DIR}/lib NO_DEFAULT_PATH)
include_directories(${jitter_BINARY_DIR}/include)

if(NOT JITTER_EXECUTABLE OR NOT JITTER_LIBRARY)
    message(FATAL_ERROR "Could not find Jitter after installation. Please check the installation process.")
endif()

message("JITTER_EXECUTABLE: ${JITTER_EXECUTABLE}")
message("JITTER_LIBRARY: ${JITTER_LIBRARY}")
message("B: ${jitter_BINARY_DIR}")

# --- Generate izmirvm-vm files ---
add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/izmirvm-vm.h ${CMAKE_SOURCE_DIR}/izmirvm-vm1.c ${CMAKE_SOURCE_DIR}/izmirvm-vm2.c ${CMAKE_SOURCE_DIR}/izmirvm-vm-main.c
    COMMAND ${JITTER_EXECUTABLE} --output ${CMAKE_SOURCE_DIR} --frontend ${CMAKE_SOURCE_DIR}/izmirvm.jitter
    DEPENDS ${CMAKE_SOURCE_DIR}/izmirvm.jitter
    VERBATIM
)

set(IZMIRVM_SHARED_SOURCES
    izmirvm-vm.h
    izmirvm-vm1.c
    izmirvm-vm2.c
    # izmirvm-vm-main.c
)

set(IZMIRVM_SOURCES
    izmirvm-vm.h
    izmirvm-vm1.c
    izmirvm-vm2.c
    izmirvm-vm-main.c)


set(IZMIRVM_MINIMAL_SOURCES
    izmirvm-vm.h
    izmirvm-vm1.c
    izmirvm-vm2.c
    izmirvm-minimal-main.c)

# --- Create the izmirvm shared library ---
# set(JITTER_LIBRARY /home/user/jitter-shared/libjitter.so)
set(JITTER_LIBRARY /home/user/izmirstuff/izmir-2.0/build/_deps/jitter-src/non-convenience-lib/.libs/libjitter.a)
set(JITTER_LIBRARY /home/user/izmirstuff/izmir-2.0/build/_deps/jitter-src/lib/.libs/libjitter.a)
# set(JITTER_LIBRARY /home/user/izmirstuff/izmir-2.0/build/_deps/jitter-src/lib/libjitter.a)
message("Create izmirvm shared - JITTER_LIBRARY: ${JITTER_LIBRARY}")
add_library(izmirvmlib SHARED ${IZMIRVM_SHARED_SOURCES})
target_link_libraries(izmirvmlib ${JITTER_LIBRARY})

# target_include_directories(izmirvmlib PRIVATE ${jitter_BINARY_DIR}/include)

set_target_properties(izmirvmlib PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    WINDOWS_EXPORT_ALL_SYMBOLS ON  # Ensure all symbols are exported
    # LINK_FLAGS -Wl,-rpath
    BUILD_WITH_INSTALL_RPATH TRUE
)

# --- Compiler flags for izmirvmlib ---
target_compile_options(izmirvmlib PRIVATE
    # -O2 -fPIC -fomit-frame-pointer -fno-reorder-functions -fno-lto -g0 -fno-var-tracking -fno-var-tracking-assignments -fno-reorder-blocks -fno-reorder-blocks-and-partition -fno-crossjumping -fno-thread-jumps -fno-tree-tail-merge -fno-isolate-erroneous-paths-dereference -fno-split-paths -fPIC -fno-align-loops -fno-align-jumps -fno-align-labels -fno-jump-tables -fno-tree-switch-conversion -flive-range-shrinkage -fno-ipa-icf -fno-ipa-cp -fno-ipa-cp-clone -mcmodel=large
    -O2 -fPIC -fomit-frame-pointer -fno-reorder-functions
    -export-dynamic
    #  -fno-lto -g0 -fno-var-tracking -fno-var-tracking-assignments -fno-reorder-blocks -fno-reorder-blocks-and-partition -fno-crossjumping -fno-thread-jumps -fno-tree-tail-merge -fno-isolate-erroneous-paths-dereference -fno-split-paths -fPIC -fno-align-loops -fno-align-jumps -fno-align-labels -fno-jump-tables -fno-tree-switch-conversion -flive-range-shrinkage -fno-ipa-icf -fno-ipa-cp -fno-ipa-cp-clone -mcmodel=large
    # -DJITTER_DISPATCH_NO_THREADING=1
)

target_compile_definitions(izmirvmlib PRIVATE
    JITTER_DISPATCH_NO_THREADING=1
)


# --- Create the izmirvm binary ---
add_executable(izmirvm_binary ${IZMIRVM_SOURCES})
target_link_libraries(izmirvm_binary ${JITTER_LIBRARY})

set_target_properties(izmirvm_binary PROPERTIES
    OUTPUT_NAME "izmirvm"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

target_compile_options(izmirvm_binary PRIVATE
    -fno-lto -g0 -fno-var-tracking -fno-var-tracking-assignments -fno-reorder-blocks -fno-reorder-blocks-and-partition -fno-crossjumping -fno-thread-jumps -fno-tree-tail-merge -fno-isolate-erroneous-paths-dereference -fno-split-paths -fPIC -fno-align-loops -fno-align-jumps -fno-align-labels -fno-jump-tables -fno-tree-switch-conversion -flive-range-shrinkage -fno-ipa-icf -fno-ipa-cp -fno-ipa-cp-clone -mcmodel=large
)

target_compile_definitions(izmirvm_binary PRIVATE
    JITTER_DISPATCH_NO_THREADING=1
)


# --- Create the minimal izmirvm binary ---
add_executable(izmirvm_minimal_binary ${IZMIRVM_MINIMAL_SOURCES})
target_link_libraries(izmirvm_minimal_binary ${JITTER_LIBRARY})

set_target_properties(izmirvm_minimal_binary PROPERTIES
    OUTPUT_NAME "izmirvm-minimal"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

target_compile_options(izmirvm_minimal_binary PRIVATE
    # -fno-lto -g0 -fno-var-tracking -fno-var-tracking-assignments -fno-reorder-blocks -fno-reorder-blocks-and-partition -fno-crossjumping -fno-thread-jumps -fno-tree-tail-merge -fno-isolate-erroneous-paths-dereference -fno-split-paths -fPIC -fno-align-loops -fno-align-jumps -fno-align-labels -fno-jump-tables -fno-tree-switch-conversion -flive-range-shrinkage -fno-ipa-icf -fno-ipa-cp -fno-ipa-cp-clone -mcmodel=large
)

target_compile_definitions(izmirvm_minimal_binary PRIVATE
    JITTER_DISPATCH_NO_THREADING=1
)
