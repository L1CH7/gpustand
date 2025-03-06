#include "JsonDirAsyncDataQueue.h"

JsonDirAsyncDataQueue::JsonDirAsyncDataQueue( const fs::path & directory, const PathsTemplate p_template, const size_t num_threads )
:   AsyncDataQueueInterface< FftData, PathsTemplate >( num_threads ),
    paths_( p_template ),
    directory_( directory )
{        
    collectData();
}

void
JsonDirAsyncDataQueue::collectData()
{
    finish_ = false;
    std::vector< fs::path > testcases_vector;
    for( auto & test_dir : fs::directory_iterator{ directory_ } )
        if( fs::is_directory( test_dir ) )
        {
#ifdef TEST_MULTIPLIER 
            // Multiplies tests and then shuffles them
            for( size_t i = 0; i < TEST_MULTIPLIER; ++i )
#endif
        // if( test_dir.path().filename()=="006")
            testcases_vector.emplace_back( test_dir.path() );
            // break; // single read
        }
#ifdef RANDOMIZE_TESTS
    std::shuffle( testcases_vector.begin(), testcases_vector.end(), std::mt19937{ std::random_device{}() } );
#endif
    BS::synced_stream ss( std::cout );
    ss.println("Collecting data...");

    size_t index = 0;
#ifdef WAIT_FOR_DIR_COUNT 
    size_t wait_for_dir_count = WAIT_FOR_DIR_COUNT;
#endif

    // auto mul_str = []( std::string s, size_t n ) -> std::string {
    //     std::string res;
    //     for( size_t i = 0; i < n; ++i ) res += s;
    //     return res;
    // };
    for( auto & testcase : testcases_vector )
    {
        ++index;
        // ++i;
        // auto task = [ testcase, this, &ss, &mul_str, i ]
        auto task = [ testcase, this, &ss ]
        { 
            // ss.println( "\r\b"s + mul_str(">"s, i) ); // incorrect progress bar...
            // std::cout << ">";
            FftParams params = readJsonParams( testcase / paths_.params_path );
            std::vector< int > mseq = readVectorFromJsonFile< int >( testcase / paths_.mseq_path );
            auto [polar0, polar1] = readVectorFromJsonFile2Polars< std::complex< int > >( testcase / paths_.data_path );
            FftData temp;
            temp.data_path = testcase;
            temp.polar = 0;
            temp.params = params;
            temp.mseq = mseq;
            temp.data_array = std::move( polar0 );
            push( std::move( temp ) );

            temp.data_path = testcase;
            temp.polar = 1;
            temp.params = params;
            temp.mseq = std::move( mseq );
            temp.data_array = std::move( polar1 );
            push( std::move( temp ) );
            // ss.println( "End read data: "s + testcase.native() );
        };
#ifdef PARALLELL_READING
        pool_.detach_task( task );
#ifdef WAIT_FOR_DIR_COUNT 
        if( index == wait_for_dir_count )
            pool_.wait();
#endif
    }
#ifndef WAIT_FOR_DIR_COUNT 
    pool_.wait();
#endif
#else
        task();
    }
#endif
    ss.println();
    ss.println("Data collected!");
    finish_ = true;
}