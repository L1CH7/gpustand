cmake_minimum_required(VERSION 3.15)

project(GpuFFT LANGUAGES CXX)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)

set(DATA_DIR OFF)

find_package(OpenCL REQUIRED)
# set(Boost_USE_STATIC_LIBS OFF) 
# set(Boost_USE_MULTITHREADED ON)  
# set(Boost_USE_STATIC_RUNTIME OFF) 
# find_package(Boost 1.87.0 COMPONENTS filesystem) 

# if(Boost_FOUND)
#     include_directories(${Boost_INCLUDE_DIRS}) 
#     # add_executable(GpuFFT file1.cxx file2.cxx) 
#     # target_link_libraries(GpuFFT ${Boost_LIBRARIES})
# endif()


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
