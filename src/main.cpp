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

    std::shared_ptr< ProgramHandler > handler = Prepare( argc, argv, root_dir / "src/fft/GpuKernels.cl" );
    if( !handler )
        exit( 1 );
    {
        RunAllTests( handler, root_dir, 10, 1, 1 );
    }
    // {   // Single test run example. All datafiles must be in same directory(test_dir)
    //     fs::path test_dir = root_dir / "testcases/FM" / "004"; // /path/to/test/dir
    //     RunSingleTest( handler, test_dir );
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

    // {
        // fs::path testcases = root_dir / "testcases/FM_copies"; // /path/to/all/testcases/dir
        // fs::path testcases = root_dir / "testcases/FM"; // /path/to/all/testcases/dir
        // auto now = std::chrono::system_clock::now();
        // auto in_time_t = std::chrono::system_clock::to_time_t( now );

        // std::stringstream ss;
        // ss << "report_" << std::put_time( std::localtime( &in_time_t ), "%Y-%m-%d_%X" );
        // fs::path report_root_dir = root_dir / "reports" / ss.str();
        // fs::path report_dir = report_root_dir / "times";
        // if( !fs::exists( report_dir ) )
        //     fs::create_directories( report_dir );
        // fs::path result_dir = report_root_dir / "data";
        // if( !fs::exists( result_dir ) )
        //     fs::create_directories( result_dir );


        // PathsTemplate data_paths{
        //     .params_path = "in_args.json",
        //     .mseq_path = "tfpMSeqSigns.json",
        //     .data_path = "out.json"
        // };
        // size_t hardware_concurrency = std::thread::hardware_concurrency();
        // size_t computing_thread_num = 1;
        // size_t read_thread_num = hardware_concurrency - computing_thread_num;

        // /**
        //  * TODO: make DataQueue asinchronous
        //  * computing thread takes data only when some is ready
        //  */
        // JsonReadDataQueue data_queue( testcases, data_paths, read_thread_num );

        // std::vector< std::shared_ptr < FftCreator > > fft_instances( computing_thread_num, nullptr );
        // BS::light_thread_pool pool( 
        //     computing_thread_num,
        //     [handler, &fft_instances]( std::size_t thread_id ){ 
        //         fft_instances.at( thread_id ) = std::make_shared< FftCreator >( handler ); 
        //     }
        // );

        // for( size_t i = 0; i < data_queue.size(); ++i )
        // // for( size_t i = 0; !data_queue.finish(); ++i )
        // // while( !data_queue.finish() )
        // {
        //     auto task = [&data_queue, &fft_instances, &report_dir, &result_dir, in_time_t, i](){
        //         // size_t thread_id = BS::this_thread::get_index().value_or(0);
        //         size_t thread_id = BS::this_thread::get_index().value_or(-1);
        //         if( thread_id > fft_instances.size() )
        //         {
        //             std::cerr << error_str("Invalid thread_id!\n");
        //             return;
        //         }

        //         // while( data_queue.empty() ) std::this_thread::sleep_for( std::chrono::milliseconds( 250 ) );

        //         auto data = data_queue.pop();
        //         FftParams params = data->params;
        //         uint8_t polar = data->polar;
        //         fs::path data_path = data->data_path;
                
        //         try
        //         {
        //             fft_instances[thread_id]->update( *data );
        //             auto time = fft_instances[thread_id]->compute();
        //             time.cpu_start_point = in_time_t;
        //             auto out_array = fft_instances[thread_id]->getFftResult();
        //             // Get output cl_float2 array ant cast it to vector

        //             std::string test_name = data->data_path.filename().native();

        //             /**
        //              * TODO: Writing DataQueue
        //              */
        //             std::stringstream ss_result;
        //             ss_result << test_name << "_result_polar" << std::to_string( polar ) 
        //                 << "_" << i << ".json";
        //             std::string data_result_name = ss_result.str();

        //             std::stringstream ss_report;
        //             ss_report << test_name << "_polar" << std::to_string( polar )
        //                 << "_" << i << ".json";
        //             std::string report_name = ss_report.str();

        //             // std::string data_result_name = test_name + "_result_polar" + std::to_string( polar ) + ".json";
        //             writeFftResultToJsonFile( result_dir / data_result_name, out_array, polar, params );
        //             // std::string report_name = test_name + "_polar" + std::to_string( polar ) + ".json";
        //             writeReportToJsonFile( report_dir / report_name, data_path, polar, params, time );
        //         }
        //         catch( const cl::Error & e )
        //         {
        //             std::stringstream ss;
        //             ss << error_str(e.what()) << std::endl;
        //             ss << error_str( getErrorString( e.err() ) ) << std::endl;
        //             ss << error_str("test#") << params.test_name << std::endl;
        //             std::cerr << ss.str();
        //             throw e;
        //         }
        //         catch( const std::exception & e )
        //         {
        //             std::cerr << error_str(e.what()) <<std::endl;
        //             throw e;
        //         }
        //     };
        //     pool.detach_task( task );
        //     // task();
        // }
        // pool.wait();
    // }

    return 0;
}