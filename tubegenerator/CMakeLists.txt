include_directories( ${OSMESA_INCLUDE_DIR} )

add_library(tubegenerator CoreTube.cpp TransferFunction.cpp)
target_link_libraries(tubegenerator geometry frame pngio OSMesa GLU)
