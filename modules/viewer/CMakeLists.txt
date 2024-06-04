project(viewer)

add_compile_options(-Wall -O3 -g -fPIC -std=c++1z)

if (DEFINED ENV{CRDC_WS})
    set(BUILD_DIR $ENV{CRDC_WS})
else ()
    set(BUILD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build/modules)
endif()

if (BUILD_DIST)
    message(STATUS "BUILD_DIST ON")
    set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/output)
else()
    message(STATUS "BUILD_DIST OFF")
    set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/build/modules)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
include_directories(BEFORE SYSTEM ${BUILD_DIR}/include)
link_directories(${BUILD_DIR}/lib)
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${BUILD_DIR}/share/cmake)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${BUILD_DIR}/share/cmake)

include_directories(BEFORE SYSTEM ${CMAKE_SYSROOT}/usr/include/aarch64-linux-gnu/ ${CMAKE_SYSROOT}/usr/include/)
find_package(OpenCV REQUIRED)

# Freetype
find_package(Freetype REQUIRED)
include_directories(${FREETYPE_INCLUDE_DIRS})

# Qt
if(${CMAKE_LIBRARY_ARCHITECTURE} STREQUAL "x86_64-linux-gnu")
 set(Qt5_ROOT_DIR /opt/qt56)		
 set(Qt5_CMAKE_DIR ${Qt5_ROOT_DIR}/lib/cmake)
endif()

list(APPEND CMAKE_PREFIX_PATH ${Qt5_CMAKE_DIR}/Qt5Widgets ${Qt5_CMAKE_DIR}/Qt5OpenGL ${Qt5_CMAKE_DIR}/Qt5Charts)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5OpenGL REQUIRED)
# find_package(Qt5Charts REQUIRED)
include_directories(SYSTEM ${Qt5Widgets_INCLUDE_DIRS} ${Qt5OpenGL_INCLUDE_DIRS} ${Qt5Charts_INCLUDE_DIRS})
link_directories(${Qt5_ROOT_DIR}/lib)

# FTGL
find_package(FTGL REQUIRED)
include_directories(SYSTEM ${FTGL_INCLUDE_DIR})

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIR})

find_package(OpenCV REQUIRED)

find_package(Eigen3 REQUIRED)
include_directories(
    ${EIGEN3_INCLUDE_DIR}
)

set(VIEWER_DEPENDS 
    # Qt5Charts 
    Qt5OpenGL Qt5Widgets Qt5Gui Qt5Core GL glut common pthread gflags glog cyber
    ${STATIC_CYBER_LIBRARIES} viewer_proto h264_rgb_encoder_decoder x264 swscale avcodec avutil
    ${PCL_LIBRARIES} ${OpenCV_LIBS} ${PROTOBUF_LIBRARIES} 
    ${PROTOBUF_PROTOC_LIBRARIES} ${FREETYPE_LIBRARIES} ${FTGL_LIBRARIES})

set(CYBER_PATH /usr/local/apollo/cyber)
link_directories(${CYBER_PATH}/lib/)
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/viewer
    ${CYBER_PATH}/include/
)
add_subdirectory(viewer)
add_subdirectory(resources)
add_subdirectory(scripts)
