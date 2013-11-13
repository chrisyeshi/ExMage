find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_DIR} ${PROJECT_SOURCE_DIR}/utilities)

add_library(particleadvector ParticleAdvector.cpp VectorFieldReader.cpp)
target_link_libraries(particleadvector ${MPI_LIBRARIES} cread mkpath procindex)
