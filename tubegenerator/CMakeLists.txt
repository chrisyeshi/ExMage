include_directories(${OSMESA_INCLUDE_DIR} ${PROJECT_SOURCE_DIR}/utilities)

add_library(tubegenerator CoreTube.cpp TransferFunction.cpp)
target_link_libraries(tubegenerator geometry frame pngio OSMesa GLU)
