set(GPU_SOURCES
    ProgramHandler.cpp
    GpuInit.cpp
)

add_library(gpu ${GPU_SOURCES})

# target_link_libraries(gpu PRIVATE fft)
# target_link_libraries(gpu PRIVATE OpenCL::OpenCL)
