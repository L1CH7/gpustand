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

target_link_libraries(${PROJECT_NAME} PRIVATE
    io_json
    gpu
    fft
    OpenCL::OpenCL
    # ${Boost_LIBRARIES}
    # BS_thread_pool
)
