cmake_minimum_required(VERSION 3.15)

project(GpuFFT LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(DATA_DIR OFF)
find_package(OpenCL REQUIRED)

include_directories(
    "${CMAKE_CURRENT_BINARY_DIR}"
    include
    src
)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/config.h"
)

add_subdirectory(src)

# add_executable(GpuFFT
#     main.cpp
#     opencl_help.h
#     ReadWriteFunctions.h ReadWriteFunctions.cpp
#     GpuFunctions.h GpuFunctions.cpp
#     GpuKernels.cl
# )

# add_library(
#     ocla
#     ocla.cpp
#     GpuFunctions.cpp)

# target_link_libraries(${PROJECT_NAME} PRIVATE ocla)



# target_link_libraries(${PROJECT_NAME} PRIVATE OpenCL::OpenCL)
# target_link_libraries(ocla PRIVATE OpenCL::OpenCL)
