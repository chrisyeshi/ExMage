cmake_minimum_required (VERSION 2.8)
project (InSitu)
cmake_policy (SET CMP0012 NEW)

#
#
# user variables
#
#
set (BUILD_SHARED ON CACHE BOOL "build shared library")
set (BUILD_EXAMPLES ON CACHE BOOL "build examples")
set (STATUS_TEXT ON CACHE BOOL "output status text while running")
set (DEBUG_TEXT OFF CACHE BOOL "output debug text while running")
find_path(OSMESA_INCLUDE_DIR osmesa.h
    /usr/local/include
    /usr/include
)
find_library(OSMESA_LIBRARY OSMesa
    /usr/local/lib
    /usr/lib
)

#
#
# shared or static library
#
#
if (${BUILD_SHARED})
	set (LIBTYPE SHARED)
else (${BUILD_SHARED})
	set (LIBTYPE STATIC)
endif (${BUILD_SHARED})

#
#
# examples
#
#
if (${BUILD_EXAMPLES})
	add_subdirectory (examples)
endif (${BUILD_EXAMPLES})

#
#
# OSMesa
#
#
include_directories (${OSMESA_INCLUDE_DIR})
link_directories (${OSMESA_LIBRARY})

#
#
# enable status text
#
#
if (${STATUS_TEXT})
	add_definitions (-DSTATUS_TEXT)
endif (${STATUS_TEXT})

#
#
# enable debug text
#
#
if (${DEBUG_TEXT})
	add_definitions (-DDEBUG_TEXT)
endif (${DEBUG_TEXT})

#
#
# include directories
#
#
include_directories (
		particleadvector
		tubegenerator
		composition
		utilities
)

#
#
# MPI
#
#
find_package (MPI REQUIRED)
include_directories (${MPI_INCLUDE_DIR})

#
#
# PNG
#
#
find_package (PNG REQUIRED)
include_directories (${PNG_INCLUDE_DIRS})

#
#
# HDF5
#
#
find_package (HDF5 REQUIRED)
include_directories (${HDF5_INCLUDE_DIRS})

#
#
# shaders
#
#
add_custom_command (
	OUTPUT color_vert.c
	COMMAND ${CMAKE_BINARY_DIR}/utilities/embedfile color_vert ${PROJECT_SOURCE_DIR}/tubegenerator/shaders/color.vert
	DEPENDS tubegenerator/shaders/color.vert
)
add_custom_command (
	OUTPUT color_frag.c
	COMMAND ${CMAKE_BINARY_DIR}/utilities/embedfile color_frag ${PROJECT_SOURCE_DIR}/tubegenerator/shaders/color.frag
	DEPENDS tubegenerator/shaders/color.frag
)

add_custom_command (
	OUTPUT normal_vert.c
	COMMAND ${CMAKE_BINARY_DIR}/utilities/embedfile normal_vert ${PROJECT_SOURCE_DIR}/tubegenerator/shaders/normal.vert
	DEPENDS tubegenerator/shaders/normal.vert
)
add_custom_command (
	OUTPUT normal_frag.c
	COMMAND ${CMAKE_BINARY_DIR}/utilities/embedfile normal_frag ${PROJECT_SOURCE_DIR}/tubegenerator/shaders/normal.frag
	DEPENDS tubegenerator/shaders/normal.frag
)

add_custom_command (
	OUTPUT scalar0_vert.c
	COMMAND ${CMAKE_BINARY_DIR}/utilities/embedfile scalar0_vert ${PROJECT_SOURCE_DIR}/tubegenerator/shaders/scalar0.vert
	DEPENDS tubegenerator/shaders/scalar0.vert
)
add_custom_command (
	OUTPUT scalar0_frag.c
	COMMAND ${CMAKE_BINARY_DIR}/utilities/embedfile scalar0_frag ${PROJECT_SOURCE_DIR}/tubegenerator/shaders/scalar0.frag
	DEPENDS tubegenerator/shaders/scalar0.frag
)

add_custom_command (
	OUTPUT scalar1_vert.c
	COMMAND ${CMAKE_BINARY_DIR}/utilities/embedfile scalar1_vert ${PROJECT_SOURCE_DIR}/tubegenerator/shaders/scalar1.vert
	DEPENDS tubegenerator/shaders/scalar1.vert
)
add_custom_command (
	OUTPUT scalar1_frag.c
	COMMAND ${CMAKE_BINARY_DIR}/utilities/embedfile scalar1_frag ${PROJECT_SOURCE_DIR}/tubegenerator/shaders/scalar1.frag
	DEPENDS tubegenerator/shaders/scalar1.frag
)

add_custom_command (
	OUTPUT scalar2_vert.c
	COMMAND ${CMAKE_BINARY_DIR}/utilities/embedfile scalar2_vert ${PROJECT_SOURCE_DIR}/tubegenerator/shaders/scalar2.vert
	DEPENDS tubegenerator/shaders/scalar2.vert
)
add_custom_command (
	OUTPUT scalar2_frag.c
	COMMAND ${CMAKE_BINARY_DIR}/utilities/embedfile scalar2_frag ${PROJECT_SOURCE_DIR}/tubegenerator/shaders/scalar2.frag
	DEPENDS tubegenerator/shaders/scalar2.frag
)

#
#
# source files
#
#
set (ROOT_SOURCES
	Explorable.cpp
)
set (PARTICLE_ADVECTOR_SOURCES
	particleadvector/ParticleAdvector.cpp
	particleadvector/PtclSync.cpp
	particleadvector/VectorFieldReader.cpp
)
set (SHADERS_SOURCES
	color_vert.c
	color_frag.c
	normal_vert.c
	normal_frag.c
	scalar0_vert.c
	scalar0_frag.c
	scalar1_vert.c
	scalar1_frag.c
	scalar2_vert.c
	scalar2_frag.c
)
set (TUBE_GENERATOR_SOURCES
	tubegenerator/CoreTube.cpp
	tubegenerator/TransferFunction.cpp
)
set (COMPOSITION_SOURCES
	composition/GlobalComposite.cpp
)
set (UTILITIES_SOURCES
	utilities/APNGWriter.cpp
	utilities/ConfigReader.cpp
	utilities/DomainInfo.cpp
	utilities/Frame.cpp
	utilities/PNGReader.cpp
	utilities/PNGWriter.cpp
	utilities/cread.c
	utilities/mkpath.cpp
)
set (SOURCES
	${ROOT_SOURCES}
	${PARTICLE_ADVECTOR_SOURCES}
	${SHADERS_SOURCES}
	${TUBE_GENERATOR_SOURCES}
	${COMPOSITION_SOURCES}
	${UTILITIES_SOURCES}
)

#
#
# build library
#
#
add_library (exmage ${LIBTYPE} ${SOURCES})
target_link_libraries (exmage ${HDF5_LIBRARIES} ${PNG_LIBRARIES} OSMesa ${MPI_LIBRARIES} GLU)