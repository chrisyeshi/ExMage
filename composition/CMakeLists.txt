find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_DIR} ${PROJECT_SOURCE_DIR}/utilities)

add_library(composition GlobalComposite.cpp)
target_link_libraries(composition ${MPI_LIBRARIES} mpngio)