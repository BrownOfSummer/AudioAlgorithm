cmake_minimum_required(VERSION 2.8)
project(ecc)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES image_registration.cpp ecc.cpp ecc.h)

add_executable(ecc ${SOURCE_FILES})
find_package(OpenCV REQUIRED)
target_link_libraries(ecc ${OpenCV_LIBRARIES})