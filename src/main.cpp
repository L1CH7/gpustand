#include <tests.h>
#include <tests_parallel.h>

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

    std::shared_ptr< ProgramHandler > handler = Prepare( argc, argv, root_dir / "src/fft/GpuKernels.cl" );
    if( !handler )
        exit( 1 );

    // FftCreator fft( handler, FftParams{ 
    //     .nl=1, 
    //     .kgrs=1,
    //     .kgd=1
    //     }, nullptr );

    // {   // Single test run example. All datafiles must be in same directory(test_dir)
    //     fs::path test_dir = root_dir / "testcases/FM" / "004"; // /path/to/test/dir
    //     RunSingleTest( fft, test_dir );
    // }

    // {   // All testcases run example. 
    //     fs::path testcases = root_dir / "testcases/FM_copies"; // /path/to/all/testcases/dir
    //     RunAllTests( fft, testcases );
    // }

    {   // All testcases parallel run example. 
        // fs::path testcases = root_dir / "testcases/FM_copies"; // /path/to/all/testcases/dir
        fs::path testcases = root_dir / "testcases/FM"; // /path/to/all/testcases/dir
        RunAllTestsParallelV4( handler, testcases );
    }


    return 0;
}