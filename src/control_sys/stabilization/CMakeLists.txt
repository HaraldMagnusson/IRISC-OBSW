cmake_minimum_required(VERSION 3.13)
project (C_FILES_SIM)

file(GLOB_RECURSE SOURCES_2 RELATIVE ${CMAKE_SOURCE_DIR} "../R2019a/*.c")
file(GLOB_RECURSE SOURCES_1 RELATIVE ${CMAKE_SOURCE_DIR} "../simple_sim_grt_rtw/*.c")
set(SOURCES_X ${SOURCES_1} ${SOURCES_2})
file(GLOB_RECURSE INCLUDES RELATIVE ${CMAKE_SOURCE_DIR} "../R2019a/*.h")

set(SOURCES ${SOURCES_X} ${INCLUDES})

list(REMOVE_ITEM SOURCES "R2019a/rtw/c/src/common/rt_main.c")

message(${SOURCES})

add_executable(c_files_sim/test test.c ${SOURCES})

target_include_directories(
        c_files_sim/test PUBLIC
        c_files_sim
        R2019a
        simple_sim_grt_rtw
        R2019a/extern/include
        R2019a/rtw/c/src
        R2019a/rtw/c/src/common
        R2019a/rtw/c/src/ext_mode/common
        R2019a/simulink/include
)

target_link_libraries(c_files_sim/test m)