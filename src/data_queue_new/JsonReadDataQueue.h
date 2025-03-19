#ifndef JSON_READ_DATA_QUEUE_H__
#define JSON_READ_DATA_QUEUE_H__

#include <ReadDataQueueInterface.h>
#include <types.h>
#include <IoJson.h>
#include <error.h>

// template< typename Filter_Tp >
// class JsonReadDataQueue : public ReadDataQueueInterface< FftData, Filter_Tp >
class JsonReadDataQueue : public ReadDataQueueInterface< FftData >
{
public:
    JsonReadDataQueue() = delete;

    JsonReadDataQueue( const size_t num_threads, const fs::path & testcases_dir, PathsTemplate paths )
    // :   ReadDataQueueInterface< FftData, Filter_Tp >( num_threads ),
    :   ReadDataQueueInterface< FftData >( num_threads ),
        paths_( paths )
    {
        if( !fs::exists( testcases_dir ) || !fs::is_directory( testcases_dir ) )
            throw std::runtime_error( "testcases_dir not exists or not directory" );

        for( const auto & entry : fs::directory_iterator{ testcases_dir } )
            if( fs::is_directory( entry ) )
                testcases_vector_.emplace_back( entry.path() );
    }
    
    JsonReadDataQueue( const size_t num_threads, const std::vector< fs::path > & testcases_vector, PathsTemplate paths )
    // :   ReadDataQueueInterface< FftData, Filter_Tp >( num_threads ),
    :   ReadDataQueueInterface< FftData >( num_threads ),
        paths_( paths ),
        testcases_vector_( testcases_vector )
    {
        for( auto path : testcases_vector_ )
            if( !fs::exists( path ) || !fs::is_directory( path ) )
                throw std::runtime_error( "one of testcases_vector paths is invalid" );
    }

    void startReading()
    {
        this->resume();
        for( auto testcase : testcases_vector_ )
        {
            auto task = [this, testcase]
            { 
                FftParams params = readJsonParams( testcase / paths_.params_path );
                std::vector< int > mseq = readVectorFromJsonFile< int >( testcase / paths_.mseq_path );
                // std::cout << "readed data! pushing....\n"s ;
                auto [polar0, polar1] = readVectorFromJsonFile2Polars< std::complex< int > >( testcase / paths_.data_path );
                // std::cout << std::to_string( this->size() ) + " - size of queue\n"s ;
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
                // std::cout << "End read data: "s << testcase.native() ;
            };
            this->pool_.detach_task( task );
        }
    }

protected:
    /**
     * used to check if reached stop condition
     */
    // bool checkStopCondition() override
    // {

    // }

    /**
     * used to filter data for reading
     */
/*
    std::optional< FftData > filterData() override
    {
        fs::path testcase;
        {
            std::scoped_lock lock( queue_mutex_ );
            if( dir_it_ == fs::directory_iterator{} || this->stop_reading_.load( std::memory_order_relaxed ) )
            {
                // stop because data ended
                // stop();
                std::cout << error_str("storing true to atomic bool + nullopt!\n");
                
                stop_reading_.store( true, std::memory_order_release );
                return std::nullopt; 
            }
            testcase = *dir_it_;
            ++dir_it_;
            if( dir_it_ == fs::directory_iterator{} )
            {
                std::cout << error_str("storing true to atomic bool!\n");
                stop_reading_.store( true, std::memory_order_release );
            }
        }
        const FftParams params = readJsonParams( testcase / paths_.params_path );
        if( this->read_filter_( params ) )
            return params;
        
        return std::nullopt;
    }
*/
    
    /**
     * read header of data. Must stop if reached end
     */
/*
    void readDataPrepare() override
    {
        fs::path testcase;
        if( testcase_it_ == testcases_vector_.cend() || this->stop_reading_.load( std::memory_order_relaxed ) )
        {
            std::cout << error_str("storing true to atomic bool + nullopt!\n");
            
            stop_reading_.store( true, std::memory_order_release );
            return; 
        }
        testcase = *testcase_it_;
        ++testcase_it_;
        if( testcase_it_ == testcases_vector_.cend() )
        {
            std::cout << error_str("storing true to atomic bool!\n");
            stop_reading_.store( true, std::memory_order_release );
        }
    }
*/
    /** 
     * read 2 polars, needs to be override
     */ 
/*
    std::optional< std::pair< FftData, FftData > > readData() override 
    {
        queue_mutex_.lock();
        fs::path testcase;
        if( testcase_it_ == testcases_vector_.cend() || this->stop_reading_.load( std::memory_order_relaxed ) )
        {
            std::cout << error_str("storing true to atomic bool + nullopt!\n");
            
            stop_reading_.store( true, std::memory_order_release );
            return std::nullopt; 
        }
        testcase = *testcase_it_;
        ++testcase_it_;
        if( testcase_it_ == testcases_vector_.cend() )
        {
            std::cout << error_str("storing true to atomic bool!\n");
            stop_reading_.store( true, std::memory_order_release );
        }
        queue_mutex_.unlock();

        std::cout << focus_str("read start:"s + testcase.native()) << '\n';

        const FftParams params = readJsonParams( testcase / paths_.params_path );
        std::vector< int > mseq = readVectorFromJsonFile< int >( testcase / paths_.mseq_path );
        auto [polar0, polar1] = readVectorFromJsonFile2Polars< std::complex< int > >( testcase / paths_.data_path );

        FftData data0;
        data0.data_path = testcase;
        data0.polar = 0;
        data0.params = params;
        data0.mseq = mseq;
        data0.data_array = std::move( polar0 );

        FftData data1;
        data1.data_path = testcase;
        data1.polar = 1;
        data1.params = params;
        data1.mseq = std::move( mseq );
        data1.data_array = std::move( polar1 );

        std::cout << focus_str("read end:"s + testcase.native()) << '\n';

        return std::make_pair( std::move( data0 ), std::move( data1 ) );
    }

    std::optional< std::pair< FftData, FftData > > readData_old() 
    {
        fs::path testcase;
        // std::cout << warn_str("readData begin\n");
        {
            std::scoped_lock lock( queue_mutex_ );
            while( dir_it_ != fs::directory_iterator{} && !fs::is_directory( *dir_it_ ) ) ++dir_it_; // skip all non-directories
            if( dir_it_ == fs::directory_iterator{} || this->stop_reading_.load( std::memory_order_relaxed ) )
            {
                // stop because data ended
                // stop();
                std::cout << error_str("storing true to atomic bool + nullopt!\n");
                
                stop_reading_.store( true, std::memory_order_release );
                return std::nullopt; 
            }
            testcase = *dir_it_;
            ++dir_it_;
            if( dir_it_ == fs::directory_iterator{} )
            {
                std::cout << error_str("storing true to atomic bool!\n");
                stop_reading_.store( true, std::memory_order_release );
            }
        }
        std::cout << focus_str("read start:"s + testcase.native()) << '\n';

        const FftParams params = readJsonParams( testcase / paths_.params_path );
        std::vector< int > mseq = readVectorFromJsonFile< int >( testcase / paths_.mseq_path );
        auto [polar0, polar1] = readVectorFromJsonFile2Polars< std::complex< int > >( testcase / paths_.data_path );

        FftData data0;
        data0.data_path = testcase;
        data0.polar = 0;
        data0.params = params;
        data0.mseq = mseq;
        data0.data_array = std::move( polar0 );

        FftData data1;
        data1.data_path = testcase;
        data1.polar = 1;
        data1.params = params;
        data1.mseq = std::move( mseq );
        data1.data_array = std::move( polar1 );

std::cout << focus_str("read end:"s + testcase.native()) << '\n';
// std::cout << focus_str("read:"s + testcase.native()) << '\n';

        return std::make_pair( std::move( data0 ), std::move( data1 ) );
    }
*/
protected:
    fs::path testcases_dir_;
    // fs::directory_iterator dir_it_;
    PathsTemplate paths_;

    std::vector< fs::path > testcases_vector_;
    // std::vector< fs::path >::const_iterator testcase_it_;
};

#endif // JSON_READ_DATA_QUEUE_H__