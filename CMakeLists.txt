cmake_minimum_required(VERSION 2.8)
project(explorable_image)

# test suites
set(BUILD_TEST ON CACHE BOOL "build the tests?")
if( BUILD_TEST )
    enable_testing()
endif( BUILD_TEST )

#status text output
set(STATUS_TEXT ON CACHE BOOL "output status text while running?")
if ( STATUS_TEXT )
	add_definitions(-DSTATUS_TEXT)
endif( STATUS_TEXT )

# debug text output
set(DEBUG_TEXT OFF CACHE BOOL "output debug text while running?")
if ( DEBUG_TEXT )
	add_definitions(-DDEBUG_TEXT)
endif( DEBUG_TEXT )

# directories that are always built
include_directories(utilities)
add_subdirectory(utilities)
add_subdirectory(test)

include_directories(particleadvector)
add_subdirectory(particleadvector)

include_directories(tubegenerator)
add_subdirectory(tubegenerator)

include_directories(composition)
add_subdirectory(composition)

# OSMesa library
find_path(OSMESA_INCLUDE_DIR osmesa.h
    /usr/local/include
    /usr/include
)
find_library(OSMESA_LIBRARY OSMesa
    /usr/local/lib
    /usr/lib
)
include_directories( ${OSMESA_INCLUDE_DIR} )
link_directories( ${OSMESA_LIBRARY} )

# only when both particleadvector and tubegenerator are built, we then build the main function
add_executable(simulate main.cpp Explorable.cpp)
target_link_libraries(simulate tubegenerator particleadvector configreader mkpath composition)
file(COPY ${PROJECT_SOURCE_DIR}/tubegenerator/shaders DESTINATION ${PROJECT_BINARY_DIR})