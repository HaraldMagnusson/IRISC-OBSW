cmake_minimum_required(VERSION 3.13)
project(irisc_obsw)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# compiler flags
set(CFLAGS "-O2 -std=gnu17 -Wall")
# useful defines: GYRO_DEBUG, GPS_DEBUG, ENCODER_DEBUG
set(COMPILE_DEFINES "")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CFLAGS} ${COMPILE_DEFINES}")

# linker flags
set(LDFLAGS "-lrt -lm -pthread -lcfitsio -lftd2xx")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${LDFLAGS}")

# list of sources
set(SCR_DIR ${CMAKE_SOURCE_DIR}/src)
file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_SOURCE_DIR} "src/*.c")
list(TRANSFORM SOURCES PREPEND "${CMAKE_SOURCE_DIR}/")

# remove certain sources
list(REMOVE_ITEM SOURCES "${SCR_DIR}/control_sys/stabilization/R2019a/rtw/c/src/common/rt_main.c")

# list of includes
file(GLOB_RECURSE INCLUDES RELATIVE ${CMAKE_SOURCE_DIR} "src/*.h")
list(TRANSFORM INCLUDES PREPEND "${CMAKE_SOURCE_DIR}/")

# remove filenames from header files to add include directories
string(REGEX REPLACE "\\/[^\\/]*\\.h" "" INCLUDES "${INCLUDES}")

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

# for debugging
message("-----SOURCES-----")
string(REPLACE ";" "\n" txt "${SOURCES}")
message(${txt})
message("-----INCLUDES----")
string(REPLACE ";" "\n" txt "${INCLUDES}")
message(${txt})

include_directories(${INCLUDES})
include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(irisc-obsw ${SOURCES})
target_link_libraries(irisc-obsw ${LIBS})
