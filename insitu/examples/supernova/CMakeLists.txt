cmake_minimum_required (VERSION 2.8)
project (Supernova)
cmake_policy (SET CMP0012 NEW)

#
#
# include directories
#
#
include_directories (
	${OSMESA_INCLUDE_DIR}
	${CMAKE_SOURCE_DIR}/insitu
	${CMAKE_SOURCE_DIR}/insitu/particleadvector
	${CMAKE_SOURCE_DIR}/insitu/tubegenerator
	${CMAKE_SOURCE_DIR}/insitu/utilities
	${CMAKE_SOURCE_DIR}/insitu/composition
)

#
#
# lib directories
#
#
link_directories (${OSMESA_LIBRARY} ${PROJECT_BINARY_DIR}/insitu)

#
#
# bulid executable
#
#
add_executable (supernova supernova.cpp)
target_link_libraries (supernova exmage)

#
#
# copy resources
#
#
file (COPY configure.json DESTINATION ${PROJECT_BINARY_DIR})
file (COPY supernova75.h5 DESTINATION ${PROJECT_BINARY_DIR})