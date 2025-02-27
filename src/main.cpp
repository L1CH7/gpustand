#include <tests.h>
#include <tests_parallel.h>

ProgramHandler * Prepare( int argc, char ** argv, fs::path kernelfile )
{
    // Choose your platform & device from `$ clinfo -l` (in my case platform_id refers to NVIDIA CUDA). 
    size_t platform_id = 0;
    size_t device_id = 0;
    switch( argc )
    {
        case 2:
            platform_id = std::stoul( argv[1] );
            device_id = 0;
            std::cout << "Warning: "_magenta << "You did not select a device number. Set to default (0)" << std::endl;
            break;
        case 3:
            platform_id = std::stoul( argv[1] );
            device_id = std::stoul( argv[2] );
            break;
        default:
        {
            std::cerr << "Wrong argc: "_red << argc << std::endl;
            std::cerr << "You must specify the platform number and (optional, default - 0) device number\n"_red;
            return nullptr;
        }
    } 
    std::string error;

    fs::path root_dir( WORKSPACE );
    ProgramHandler * handler = makeProgramHandler( platform_id, device_id, error );
    if( error != "")
    {
        std::cerr << "ERROR: "_red << error << std::endl;
        return nullptr;
    }
    auto identity = initGpuModule( handler, kernelfile );
    return handler;
}

int main( int argc, char ** argv )
{
    fs::path root_dir( WORKSPACE );

    ProgramHandler * handler = Prepare( argc, argv, root_dir / "src/fft/GpuKernels.cl" );
    if( !handler )
        exit( 1 );

    FftCreator fft( handler, FftParams{ 
        .nl=1, 
        .kgrs=1,
        .kgd=1
        }, nullptr );

    {   // Single test run example. All datafiles must be in same directory(test_dir)
        fs::path test_dir = root_dir / "testcases/FM" / "000"; // /path/to/test/dir
        RunSingleTest( fft, test_dir );
    }
    {   // Single test run example. All datafiles must be in same directory(test_dir)
        fs::path test_dir = root_dir / "testcases/AM" / "000"; // /path/to/test/dir
        RunSingleTest( fft, test_dir );
    }
    // {   // Single test run example. All datafiles must be in same directory(test_dir)
    //     fs::path test_dir = root_dir / "testcases/AM" / "048"; // /path/to/test/dir
    //     RunSingleTest( fft, test_dir );
    // }
    // {   // Single test run example. All datafiles must be in same directory(test_dir)
    //     fs::path test_dir = root_dir / "testcases/AM" / "001"; // /path/to/test/dir
    //     RunSingleTest( fft, test_dir );
    // }
    // // delete handler;return 0;
    // {   // Single test run example. All datafiles must be in same directory(test_dir)
    //     fs::path test_dir = root_dir / "testcases/FM" / "002"; // /path/to/test/dir
    //     RunSingleTest( fft, test_dir );
    // }

    // {   // All testcases run example. 
    //     fs::path testcases = root_dir / "testcases/AM"; // /path/to/all/testcases/dir
    //     RunAllTests( fft, testcases );
    // }

    // {   // All testcases run example. 
    //     fs::path testcases = root_dir / "testcases/AM"; // /path/to/all/testcases/dir
    //     RunAllTestsParallelV2( handler, testcases );
    // }
    delete handler;

    return 0;
}