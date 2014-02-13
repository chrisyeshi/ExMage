cmake_minimum_required(VERSION 2.8)
project(ExMage)
cmake_policy (SET CMP0012 NEW)

#
#
# user variables
#
#
set (BUILD_TEST ON CACHE BOOL "build the tests")

#
#
# utilities
#
#
add_subdirectory (utilities)

#
#
# enable tests
#
#
if (${BUILD_TEST})
	add_subdirectory (test)
	enable_testing ()
endif (${BUILD_TEST})

#
#
# add subdirectories
#
#
add_subdirectory (insitu)