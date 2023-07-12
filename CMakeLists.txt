cmake_minimum_required(VERSION 3.0)
project(Execute_File)
include_directories( ${OpenCV_INCLUDE_DIRS} )
find_package( OpenCV REQUIRED )
# set for compiler 
set(CMAKE_CXX_COMPILER g++)

# set CUDA
find_package(CUDA)
include_directories("${CUDA_INCLUDE_DIRS}")
add_definitions(-DUSE_CUDA)

# set for debug
set(CMAKE_BUILD_TYPE Debug)
# set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/Src/VideoCap_2cam.cpp)
# set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/Src/sift_ransac_ud.cpp)
set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/Src/VideoCapture.cpp)
add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries( ${PROJECT_NAME} ${OpenCV_LIBS} "stdc++fs")



