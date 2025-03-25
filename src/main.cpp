// #include <tests.h>
// #include <tests_parallel.h>

#include <config.h>
#include <CLDefs.h>
#include <GpuFourier.h>
#include <GpuInit.h>
#include <error.h>
#include <JsonReadDataQueue.h>
#include <JsonWriteDataQueue.h>
#include <BS_thread_pool.hpp>

#include <tests.h>

std::shared_ptr< ProgramHandler > Prepare( int argc, char ** argv, fs::path kernel_path )
{
    // Choose your platform & device from `$ clinfo -l` (in my case platform_id refers to NVIDIA CUDA). 
    size_t platform_id = 0;
    size_t device_id = 0;
    switch( argc )
    {
        case 2:
            platform_id = std::stoul( argv[1] );
            device_id = 0;
            std::cout << warn_str( "Warning: You did not select a device number. Set to default (0)" ) << std::endl;
            break;
        case 3:
            platform_id = std::stoul( argv[1] );
            device_id = std::stoul( argv[2] );
            break;
        default:
        {
            std::cerr << error_str( "Wrong argc: " ) << argc << std::endl;
            std::cerr << error_str( "You must specify the platform number and (optional, default - 0) device number\n" );
            return nullptr;
        }
    } 

    std::string error;
    std::shared_ptr< ProgramHandler > handler = makeProgramHandler( platform_id, device_id, error );
    if( error != "")
    {
        std::cerr << error_str( "ERROR: " + error ) << std::endl;
        return nullptr;
    }
    auto identity = initGpuModule( handler, kernel_path );
    std::cout << focus_str( "Platform: " + identity.platform_name + "\nDevice: " + identity.device_name ) << std::endl;
    return handler;
}

int main( int argc, char ** argv )
{
    fs::path root_dir( WORKSPACE );

    size_t hardware_concurrency = std::thread::hardware_concurrency();
    size_t read_thread_num = 6;
    size_t computing_thread_num = 1;
    size_t write_thread_num = 5;

    std::shared_ptr< ProgramHandler > handler = Prepare( argc, argv, root_dir / "src/fft/GpuKernels.cl" );
    if( !handler )
        exit( 1 );
    {
        // RunAllTests( handler, root_dir, 10, 1, 1 );
    }
    {
        testExecutionPool( handler, root_dir, read_thread_num, computing_thread_num, write_thread_num );
    }

    return 0;
}