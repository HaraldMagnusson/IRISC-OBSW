cmake_minimum_required(VERSION 3.13)
project(irisc_obsw)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# compiler flags
set(CFLAGS "-O2 -std=gnu17 -Wall")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CFLAGS}")

# linker flags
set(LDFLAGS "-lrt -pthread -lcfitsio")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${LDFLAGS}")

# list of sources
set(SCR_DIR ${CMAKE_SOURCE_DIR}/src)
set(SOURCES
	${SCR_DIR}/camera/camera.c
	${SCR_DIR}/camera/camera_utils/camera_utils.c
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
	${SCR_DIR}/tracking/tracking.c
	${SCR_DIR}/tracking/controller/controller.c
	${SCR_DIR}/tracking/current_target/current_target.c
	${SCR_DIR}/tracking/gimbal/gimbal.c
	${SCR_DIR}/tracking/target_selecting/target_selecting.c
	${SCR_DIR}/watchdog/watchdog.c
)

# list of libraries
set(LIB_DIR ${CMAKE_SOURCE_DIR}/lib)
set(LIBS
    ${LIB_DIR}/libASICamera2.so
)

#remove filenames from source files to add include directories
set(DIRS ${SOURCES})
string(REGEX REPLACE "\\/[^\\/]*\\.c" "" DIRS "${DIRS}")
include_directories(${DIRS})
include_directories(${CMAKE_SOURCE_DIR}/include/)

add_executable(irisc-obsw ${SOURCES})
target_link_libraries(irisc-obsw ${LIBS})
