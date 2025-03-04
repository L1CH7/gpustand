include_directories(
    # tests
    io_json
    gpu
    fft
    data_queue
)

add_subdirectory(io_json)
add_subdirectory(gpu)
add_subdirectory(fft)
add_subdirectory(data_queue)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE
    main.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    # tests
    io_json
    gpu
    fft
    data_queue
    OpenCL::OpenCL
)
