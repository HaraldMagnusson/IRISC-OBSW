cmake_minimum_required(VERSION 3.13)
project(irisc_obsw)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# compiler flags
set(CFLAGS "-O2 -std=gnu17 -Wall -Wpedantic")

# useful debug defines: GYRO_DEBUG, GPS_DEBUG, ENCODER_DEBUG,
#                       SEQ_DEBUG, ST_DEBUG, E_LINK_DEBUG, DOWNLINK_DEBUG
#                       CAMERA_DEBUG
set(COMPILE_DEFINES "-DST_DEBUG -DSEQ_DEBUG -DE_LINK_DEBUG -DDOWNLINK_DEBUG -DCAMERA_DEBUG")

#useful test defines: ST_TEST, SEQ_TEST
set(COMPILE_DEFINES "${COMPILE_DEFINES} -DSEQ_TEST")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CFLAGS} ${COMPILE_DEFINES}")

# linker flags
set(LDFLAGS "-lrt -lm -pthread -lcfitsio -lftd2xx -lzstd")
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
	${SCR_DIR}/e_link/e_link.c
	${SCR_DIR}/global_utils/global_utils.c
	${SCR_DIR}/gpio/gpio.c
	${SCR_DIR}/img_processing/img_processing.c
	${SCR_DIR}/img_processing/data_queue/data_queue.c
	${SCR_DIR}/img_processing/image_handler/image_handler.c
	${SCR_DIR}/init/init.c
	${SCR_DIR}/mode/mode.c
	${SCR_DIR}/sensors/encoder/encoder.c
	${SCR_DIR}/sensors/gps/gps.c
	${SCR_DIR}/sensors/sensors.c
	${SCR_DIR}/sensors/gyroscope/gyroscope.c
	${SCR_DIR}/sensors/sensor_poller/encoder_poller/encoder_poller.c
	${SCR_DIR}/sensors/sensor_poller/gps_poller/gps_poller.c
	${SCR_DIR}/sensors/sensor_poller/gyroscope_poller/gyroscope_poller.c
	${SCR_DIR}/sensors/sensor_poller/sensor_poller.c
	${SCR_DIR}/sensors/sensor_poller/star_tracker_poller/star_tracker_poller.c
	${SCR_DIR}/sensors/sensor_poller/temperature_poller/temperature_poller.c
	${SCR_DIR}/sensors/star_tracker/star_tracker.c
	${SCR_DIR}/sensors/temperature/temperature.c
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
string(COMPARE EQUAL ${CMAKE_SYSTEM_PROCESSOR} "armv7l" strcmp)
if(strcmp)
	set(LIB_DIR ${CMAKE_SOURCE_DIR}/lib_arm)
else()
        set(LIB_DIR ${CMAKE_SOURCE_DIR}/lib)
endif(strcmp)
unset(strcmp CACHE)

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
