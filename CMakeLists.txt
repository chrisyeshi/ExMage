cmake_minimum_required(VERSION 2.8)
project(explorable_image)

# What to build?
set(BUILD_PARTICLE_TRACER ON CACHE BOOL "whether to build the particle tracer library")
set(BUILD_TUBE_GENERATOR ON CACHE BOOL "whether to build the tube generator library")

# directories that are always built
include_directories(geometry utilities)
add_subdirectory(geometry)
add_subdirectory(utilities)
add_subdirectory(test)

if( BUILD_PARTICLE_TRACER )
    include_directories(particletracer)
    add_subdirectory(particletracer)
endif( BUILD_PARTICLE_TRACER )

if( BUILD_TUBE_GENERATOR )
    include_directories(tubegenerator)
    add_subdirectory(tubegenerator)
endif( BUILD_TUBE_GENERATOR )

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

# only when both particletracer and tubegenerator are built, we then build the main function
if( BUILD_PARTICLE_TRACER AND BUILD_TUBE_GENERATOR )
    add_executable(simulate main.cpp)
    target_link_libraries(simulate geometry tubegenerator particletracer configreader mkpath)
    file(COPY ${PROJECT_SOURCE_DIR}/tubegenerator/shaders ${PROJECT_SOURCE_DIR}/configure.json DESTINATION ${PROJECT_BINARY_DIR})
endif( BUILD_PARTICLE_TRACER AND BUILD_TUBE_GENERATOR )
