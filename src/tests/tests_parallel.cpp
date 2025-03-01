#include "tests_parallel.h"

void RunAllTestsParallel( FftCreator & fft, const fs::path & testcases_dir )
{
    // if( !handler )
    // {
    //     std::cout << "No program handler created for tests\n"_red;
    // }

    BS::thread_pool pool;
    // fs::path p; // local wariable should save after for-loop quit
    for( auto & testcase : fs::directory_iterator{ testcases_dir } )
    {
        if( fs::is_directory( testcase ) )
        {
            fs::path p = testcase.path();
            // pool.submit_task( [ &handler, &testcase ]{ 
            // std::this_thread::sleep_for(std::chrono::milliseconds(500<<2));
            pool.detach_task( [ &fft, p ]{ 
                RunSingleTest( fft, p ); 
            } );
        }

    }
    pool.wait();
}

void RunAllTestsParallelV2( std::shared_ptr< ProgramHandler > handler, const fs::path & testcases_dir )
{
    if( !handler )
    {
        std::cout << "No program handler created for tests\n"_red;
    }

    BS::thread_pool pool;
    for( auto & testcase : fs::directory_iterator{ testcases_dir } )
    {
        if( fs::is_directory( testcase ) )
        {

            fs::path p = testcase.path();
            // wait if all threads busy
            while( pool.get_tasks_running() >= pool.get_thread_count() ){};

            pool.detach_task( [ &handler, p ]{ 
                FftCreator fft{ handler, FftParams{ 
                    .is_am = false,
                    .nl=1, 
                    .kgrs=1,
                    .kgd=1
                    }, nullptr };
                RunSingleTest( fft, p ); 
            } );
        }
    }
    pool.wait();
}

// #include <TaskQueue.h>

// class Task
// {
//     void operator()()
//     {
//         std::this_thread::sleep_for(std::chrono::seconds(2));
//         std::cout << "penis\n";
//         std::this_thread::sleep_for(std::chrono::seconds(2));
//     }
// };
// void RunAllTestsParallelV3( std::shared_ptr< ProgramHandler > handler, const fs::path & testcases_dir )
// {
//     size_t threads_size = 22;
//     TaskQueue< Task > WorkQueue( 12 );
//     Task tasks[threads_size];
//     for( size_t i = 0; i < threads_size; ++i )
//     {
//         WorkQueue.Enqueue( tasks[i] );
//     }
// }

void RunAllTestsParallelV3( std::shared_ptr< ProgramHandler > handler, const fs::path & testcases_dir )
{
    if( !handler )
    {
        std::cout << "No program handler created for tests\n"_red;
    }

    BS::thread_pool pool;
    for( auto & testcase : fs::directory_iterator{ testcases_dir } )
    {
        if( fs::is_directory( testcase ) )
        {

            fs::path test_dir = testcase.path();
            // wait if all threads busy
            // while( pool.get_tasks_running() >= pool.get_thread_count() ){};

            pool.detach_task( [ &handler, test_dir ]{ 
                FftCreator fft{ handler, FftParams{ 
                    .is_am = false,
                    .nl=1,
                    .kgrs=1,
                    .kgd=1
                }, nullptr };
                std::cout << "test:" << test_dir.c_str() << '\n';

                fs::path result_dir     = test_dir      / "result";
                if( !fs::exists( result_dir ) )
                    fs::create_directory( result_dir );

                Paths paths
                {
                    .params_path        = test_dir      / "in_args.json",
                    .input_path         = test_dir      / "out.json",
                    .mseq_path          = test_dir      / "tfpMSeqSigns.json",
                    .result_data_path   = result_dir    / "data.json",
                    .result_time_path   = result_dir    / "time.out"
                };
                FftParams params = readJsonParams( paths.params_path );
                params.shgd *= params.ndec;
                if( params.is_am )
                {
                    std::cout << "AM!!!\n";
                    params.log2N = ( uint32_t )std::log2( params.true_nihs ) + 1;
                    params.mseq = std::vector< int >();
                }
                else
                {
                    std::cout << "FM!!!\n";
                    params.log2N = ( uint32_t )std::ceil( std::log2( ( double )params.dlstr / params.ndec ) );
                    params.mseq = readVectorFromJsonFile< cl_int >( paths.mseq_path );
                    params.mseq.resize( 1 << params.log2N, 0 ); // mseq should be N elems, filled with zeroes
                }

                // Read input vector and cast it to cl_int2
                auto [ polar0, polar1 ] = readVectorFromJsonFile2Polars< std::complex< int > >( paths.input_path );
                auto polar0_ptr = reinterpret_cast< cl_int2 * >( polar0.data() );
                auto polar1_ptr = reinterpret_cast< cl_int2 * >( polar1.data() );

                // FftCreator fft( handler, params, polar0_ptr );
                fft.update( params, polar0_ptr );
                auto times0 = fft.compute();
                auto res0_ptr = fft.getFftResult();

                fft.update( params, polar1_ptr );
                auto times1 = fft.compute();
                auto res1_ptr = fft.getFftResult();


                // Get output cl_float2 array ant cast it to vector
                size_t res_size = params.nl * params.kgd * params.kgrs;
                auto res0_complex_ptr = reinterpret_cast< std::complex< float > * >( res0_ptr );
                auto res1_complex_ptr = reinterpret_cast< std::complex< float > * >( res1_ptr );

                json j_out;
                std::vector< std::vector< std::complex< float > > > resv0rays( params.nl );
                std::vector< std::vector< std::complex< float > > > resv1rays( params.nl );
                for( size_t i = 0, offset = 0, step = res_size / params.nl; i < params.nl; ++i, offset += step )
                {
                    std::stringstream key0, key1;
                    resv0rays[i] = std::vector< std::complex< float > >( res0_complex_ptr + offset, res0_complex_ptr + offset + step );
                    key0 << "Ray" << i << "Polar0";
                    j_out[key0.str()] = resv0rays[i];

                    resv1rays[i] = std::vector< std::complex< float > >( res1_complex_ptr + offset, res1_complex_ptr + offset + step );
                    key1 << "Ray" << i << "Polar1";
                    j_out[key1.str()] = resv1rays[i];
                }

                std::ofstream ofs( paths.result_data_path );
                ofs << j_out.dump(4);
                writeTimeStampsToJsonFile( paths.result_time_path.parent_path() / "time0.json", times0 );
                writeTimeStampsToJsonFile( paths.result_time_path.parent_path() / "time1.json", times1 ); 
            } );
        }
    }
    pool.wait();
}

void RunAllTestsParallelV4( std::shared_ptr< ProgramHandler > handler, const fs::path & testcases_dir )
{
    size_t threads_size = 22;
    BS::thread_pool pool;
    size_t i = 0;
    std::mutex _mut;
    pool.wait();
    for( i = 0; i < threads_size; ++i )
    {
        // while( pool.get_tasks_running() >= pool.get_thread_count() ){};
        pool.detach_task( [&_mut,i](  ){ 
            {
                std::lock_guard< std::mutex > _lock( _mut );
                std::cout << "penis #" << i << '\n';
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(2500));
        } );
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    std::cout << "12 done!\n";
    // std::this_thread::sleep_for(std::chrono::milliseconds(500));
    // std::cout << "24 done!\n";
    // std::this_thread::sleep_for(std::chrono::milliseconds(500));
    // std::cout << "36 done!\n";
    pool.wait();
    // pool.purge();
    // pool.submit_sequence(0,7,[&_mut]( const size_t & i ){ 
    //         std::lock_guard< std::mutex > _lock( _mut );
    //         std::cout << "penis #" << i << '\n';
    //     } ).wait();

}
