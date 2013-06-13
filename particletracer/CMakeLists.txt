find_package(MPI REQUIRED)
include_directories( ${MPI_INCLUDE_DIR} )

add_library(particletracer Simulator.cpp VectorFieldReader.cpp)
target_link_libraries(particletracer ${MPI_LIBRARIES} cread mkpath procindex)
