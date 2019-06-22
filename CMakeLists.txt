cmake_minimum_required(VERSION 3.13)
project(irisc_obsw)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# compiler flags
set(CFLAGS "-O2 -std=gnu11 -Wall")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CFLAGS}")

# linker flags
set(LDFLAGS "-lrt -lm -pthread")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${LDFLAGS}")

# list of sources
set(SCR_DIR ${CMAKE_SOURCE_DIR}/src)
set(SOURCES 
    ${SCR_DIR}/camera/camera.c
    ${SCR_DIR}/camera/camera_control/camera_control.c
    ${SCR_DIR}/camera/guiding_camera/guiding_camera.c
    ${SCR_DIR}/camera/nir_camera/nir_camera.c
    ${SCR_DIR}/camera/sanity_camera/sanity_camera.c
    ${SCR_DIR}/command/command.c
    ${SCR_DIR}/data_storage/data_storage.c
    ${SCR_DIR}/e_link/e_link.c
    ${SCR_DIR}/global_utils/global_utils.c
    ${SCR_DIR}/i2c/i2c.c
    ${SCR_DIR}/img_processing/img_processing.c
    ${SCR_DIR}/img_processing/data_queue/data_queue.c
    ${SCR_DIR}/img_processing/image_handler/image_handler.c
    ${SCR_DIR}/init/init.c
    ${SCR_DIR}/mode/mode.c
    ${SCR_DIR}/sensors/sensors.c
    ${SCR_DIR}/sensors/orientation/orientation.c
    ${SCR_DIR}/sensors/sensor_poller/sensor_poller.c
    ${SCR_DIR}/sensors/sun/sun.c
    ${SCR_DIR}/sensors/temperature/temperature.c
    ${SCR_DIR}/spi/spi.c
    ${SCR_DIR}/telemetry/telemetry.c
    ${SCR_DIR}/telemetry/downlink/downlink.c
    ${SCR_DIR}/telemetry/downlink_queue/downlink_queue.c
    ${SCR_DIR}/thermal/thermal.c
    ${SCR_DIR}/control_sys/control_sys.c
    ${SCR_DIR}/control_sys/stabilization/stabilization.c
    ${SCR_DIR}/control_sys/current_target/current_target.c
    ${SCR_DIR}/control_sys/gimbal/gimbal.c
    ${SCR_DIR}/control_sys/target_selection/target_selection.c
    ${SCR_DIR}/watchdog/watchdog.c
)

#list of sources for exported simulink model
set(SIM_DIR ${CMAKE_SOURCE_DIR}/src/control_sys/stabilization)
message(${SIM_DIR})
set(SIMULINK
    ${SIM_DIR}/R2019a
    ${SIM_DIR}/simple_sim_grt_rtw
    ${SIM_DIR}/R2019a/extern/include
    ${SIM_DIR}/R2019a/rtw/c/src
    ${SIM_DIR}/R2019a/rtw/c/src/common
    ${SIM_DIR}/R2019a/rtw/c/src/ext_mode/common
    ${SIM_DIR}/R2019a/simulink/include
)

file(GLOB_RECURSE SOURCES_2 RELATIVE ${CMAKE_SOURCE_DIR} "${SIM_DIR}/R2019a/*.c")
file(GLOB_RECURSE SOURCES_1 RELATIVE ${CMAKE_SOURCE_DIR} "${SIM_DIR}/simple_sim_grt_rtw/*.c")
file(GLOB_RECURSE INCLUDES RELATIVE ${CMAKE_SOURCE_DIR} "${SIM_DIR}/R2019a/*.h")

#set(SRC ${SOURCES_1} ${SOURCES_2})
set(SOURCES_X ${SOURCES_1} ${SOURCES_2})
set(SRC ${SOURCES_X} ${INCLUDES})
list(REMOVE_ITEM SRC "src/control_sys/stabilization/R2019a/rtw/c/src/common/rt_main.c")
list(TRANSFORM SRC PREPEND "${CMAKE_SOURCE_DIR}/")

#remove filenames from source files to add include directories
set(DIRS ${SOURCES})
string(REGEX REPLACE "\\/[^\\/]*\\.c" "" DIRS "${DIRS}")
set(DIRS ${DIRS} ${SIMULINK})
set(SOURCES ${SOURCES} ${SRC})

# for debugging
message("-----SOURCES-----")
string(REPLACE ";" "\n" txt "${SOURCES}")
message(${txt})
message("-----INCLUDES----")
string(REPLACE ";" "\n" txt "${DIRS}")
message(${txt})

add_executable(irisc-obsw ${SOURCES})

target_include_directories(irisc-obsw PUBLIC ${DIRS})
