cmake_minimum_required(VERSION 2.8)
project(explorable_image)

# test suites
set(BUILD_TEST ON CACHE BOOL "build the tests?")
if( BUILD_TEST )
    enable_testing()
endif( BUILD_TEST )

# directories that are always built
include_directories(utilities)
add_subdirectory(utilities)
add_subdirectory(test)

include_directories(particleadvector)
add_subdirectory(particleadvector)

include_directories(tubegenerator)
add_subdirectory(tubegenerator)

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
target_link_libraries(simulate tubegenerator particleadvector configreader mkpath)
file(COPY ${PROJECT_SOURCE_DIR}/tubegenerator/shaders DESTINATION ${PROJECT_BINARY_DIR})
