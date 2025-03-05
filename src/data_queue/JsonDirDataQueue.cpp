#include "JsonDirDataQueue.h"

JsonDirDataQueue::JsonDirDataQueue( const fs::path & directory, const PathsTemplate p_template, const size_t num_threads )
:   paths_( p_template ),
    directory_( directory ),
    pool_( num_threads )
{        
    collectData();
}

void
JsonDirDataQueue::collectData()
{
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
    // std::uniform_int_distribution< size_t > distrib( 0UL, testcases_vector.size() - 1 );
    // std::uniform_int_distribution< size_t > distrib();
    std::shuffle( testcases_vector.begin(), testcases_vector.end(), std::mt19937{ std::random_device{}() } );
#endif
    BS::synced_stream ss( std::cout );
    for( auto & testcase : testcases_vector )
    {
        auto task = [ testcase, this, &ss ](){ 
            // ss.println( "Reading data: "s + testcase.native() );
            FftParams params = readJsonParams( testcase / paths_.params_path );
            std::vector< int > mseq = readVectorFromJsonFile< int >( testcase / paths_.mseq_path );
            // auto mseq0 = readVectorFromJsonFile< int >( testcase / paths_.mseq_path );
            // std::vector< int > mseq1 = mseq0;
            // std::copy( mseq0.begin(), mseq0.end(), mseq1 );
            auto [polar0, polar1] = readVectorFromJsonFile2Polars< std::complex< int > >( testcase / paths_.data_path );
            // ss.println( "log: "s + testcase.native() );
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
            // push( FftData{
            //     testcase,
            //     0,
            //     params,
            //     mseq, // copy assignment // std::copy( mseq.begin(), mseq.end() ),
            //     std::move( polar0 )
            // });
            // push( FftData{
            //     testcase,
            //     1,
            //     params,
            //     std::move( mseq ),
            //     std::move( polar1 )
            // });
            ss.println( "End read data: "s + testcase.native() );
        };
#ifdef PARALLELL_READING
        pool_.detach_task( task );
    }
    pool_.wait();
#else
        task();
    }
#endif
}