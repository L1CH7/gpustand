// #include <tests.h>
// #include <tests_parallel.h>

// #include <types.h>
#include <CLDefs.h>
#include <GpuFourier.h>
#include <GpuInit.h>
// #include <IoJson.h>
// #include <config.h>
#include <error.h>
#include <JsonDirDataQueue.h>
#include <BS_thread_pool.hpp>

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

    // {   // All testcases parallel run example. 
    //     // fs::path testcases = root_dir / "testcases/FM_copies"; // /path/to/all/testcases/dir
    //     fs::path testcases = root_dir / "testcases/FM"; // /path/to/all/testcases/dir
    //     RunAllTestsParallelV4( handler, testcases );
    // }

    {
        // fs::path testcases = root_dir / "testcases/FM_copies"; // /path/to/all/testcases/dir
        fs::path testcases = root_dir / "testcases/FM"; // /path/to/all/testcases/dir
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << "report_" << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d_%X");
        fs::path report_dir = root_dir / "testcases" / ss.str();
        if( !fs::exists( report_dir ) )
            fs::create_directories( report_dir );


        PathsTemplate data_paths{
            .params_path = "in_args.json",
            .mseq_path = "tfpMSeqSigns.json",
            .data_path = "out.json"
        };
        size_t read_thread_num = 4;

        JsonDirDataQueue data_queue( testcases, data_paths, read_thread_num );

        size_t computing_thread_num = std::thread::hardware_concurrency() - read_thread_num;
        std::vector< std::shared_ptr < FftCreator > > fft_instances( computing_thread_num, nullptr );
        BS::light_thread_pool pool( 
            computing_thread_num,
            [handler, &fft_instances]( std::size_t thread_id ){ 
                FftParams dummy_params{.is_am=false,.nl=1,.kgrs=1,.kgd=1};
                fft_instances.at( thread_id ) = std::make_shared< FftCreator >( handler, dummy_params, nullptr ); 
            }
        );



        for( size_t i = 0; i < data_queue.size(); ++i )
        {
            auto task = [&data_queue, &fft_instances, &report_dir](){
                size_t thread_id = BS::this_thread::get_index().value_or(-1);
                if( thread_id == -1 )
                    throw error_str("Invalid thread_id!\n");

                auto data = data_queue.pop();
                auto polar_ptr = reinterpret_cast< cl_int2 * >( data->data.data() );
                data->params.mseq = std::move( data->mseq );

                fft_instances[thread_id]->update( data->params, polar_ptr );
                auto time = fft_instances[thread_id]->compute();
                auto result = fft_instances[thread_id]->getFftResult();

                std::string report_name = data->data_path.filename().native() + "_polar" + std::to_string( data->polar ) + ".json";
                writeReportToJsonFile( report_dir / report_name, data->data_path, data->polar, data->params, time );
            };
            pool.detach_task( task );
        }
        pool.wait();
    }

    return 0;
}