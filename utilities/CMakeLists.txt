# procindex
add_library(procindex ProcIndex.cpp)

# mkpath
add_library(mkpath mkpath.cpp)

# configreader
add_library(configreader ConfigReader.cpp)

# frame
add_library(frame Frame.cpp)

# pngio
include( ${CMAKE_ROOT}/Modules/FindPNG.cmake )

if(PNG_FOUND)
    include_directories(${PNG_INCLUDE_DIRS})
endif(PNG_FOUND)

add_library(pngio PNGReader.cpp PNGWriter.cpp)
target_link_libraries(pngio ${PNG_LIBRARIES})

add_library(mpngio APNGWriter.cpp)
target_link_libraries(mpngio ${PNG_LIBRARIES})

# cread
find_package(HDF5 REQUIRED)
include_directories(${HDF5_INCLUDE_DIRS})

add_library(cread cread.c)
target_link_libraries(cread ${HDF5_LIBRARIES})
