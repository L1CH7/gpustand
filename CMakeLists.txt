cmake_minimum_required(VERSION 3.15)

project(GpuFFT LANGUAGES CXX)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)

find_package(OpenCL REQUIRED)

include_directories(
    "${CMAKE_CURRENT_BINARY_DIR}"
    include
    src
)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/config.hxx.in"
    "${CMAKE_CURRENT_BINARY_DIR}/config.hxx"
)

add_subdirectory(src)
