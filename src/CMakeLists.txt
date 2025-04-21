include_directories(
    tests
    io_json
    gpu
    fft
    data_queue
    execution_pool
    data_hasher
)

add_subdirectory(io_json)
add_subdirectory(gpu)
add_subdirectory(fft)
add_subdirectory(tests)
add_subdirectory(execution_pool)
add_subdirectory(data_hasher)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE
    main.cxx
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    tests
    io_json
    gpu
    fft
    execution_pool
    data_hasher
    OpenCL::OpenCL
)
