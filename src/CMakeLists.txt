include_directories(
    io_json
    gpu
    fft
)

add_subdirectory(io_json)
add_subdirectory(gpu)
add_subdirectory(fft)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE
    main.cpp
)

target_link_libraries(${PROJECT_NAME} PUBLIC
    io_json
    OpenCL::OpenCL
    gpu
    fft
)
