cmake_minimum_required(VERSION 3.5)

project(Converter LANGUAGES CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories("./include")
include_directories("./include/xassert.h")
include_directories("./include/pixel_rgb888.h")
include_directories("./include/BMP_Image.h")
include_directories("./include/yuv420_frame.h")

add_executable(Converter main.cpp yuv420_frame.cpp BMP_Image.cpp pixel_rgb888.cpp xassert.cpp)
