#ifndef TESTS_PARALLEL_H__
#define TESTS_PARALLEL_H__

#include <tests.h>
#include <thread>

#include <BS_thread_pool.hpp>

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
void RunAllTestsParallelV2( ProgramHandler * handler, const fs::path & testcases_dir )
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

#endif // TESTS_PARALLEL_H__