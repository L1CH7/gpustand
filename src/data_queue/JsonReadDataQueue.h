#ifndef JSON_READ_DATA_QUEUE_H__
#define JSON_READ_DATA_QUEUE_H__

#include <ReadDataQueueInterface.h>
// #include <types.h>
#include <IoJson.h>
#include <error.h>


struct ReadPathsTemplate
{
    fs::path params_path;
    fs::path mseq_path;
    fs::path data_path;
};

// template< typename Filter_Tp >
// class JsonReadDataQueue : public ReadDataQueueInterface< FftData, Filter_Tp >
class JsonReadDataQueue : public ReadDataQueueInterface< FftData >
{
public:
    JsonReadDataQueue() = delete;

    JsonReadDataQueue( const size_t num_threads, const fs::path & testcases_dir, ReadPathsTemplate paths )
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
    
    JsonReadDataQueue( const size_t num_threads, const std::vector< fs::path > & testcases_vector, ReadPathsTemplate paths )
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
                readPushTwoPolars( testcase );
            };
            this->pool_.detach_task( task );
        }
    }

    void startReadingSplitKGRS( size_t step )
    {
        this->resume();
        for( auto testcase : testcases_vector_ )
        {
            auto task = [this, testcase, step]
            { 
                readPushTwoPolarsSplitKGRS( testcase, step );
            };
            this->pool_.detach_task( task );
        }
        std::cout << focus_str("all test read split kgrs\n");
    }

private:
    void readPushTwoPolars( fs::path testcase_path )
    {
        FftParams params = IoJson::readParams( testcase_path / this->paths_.params_path );
        std::vector< int > mseq = IoJson::readMseq( testcase_path / this->paths_.mseq_path );
        auto [polar0, polar1] = IoJson::readStrobe( testcase_path / this->paths_.data_path );

        FftData temp;
        temp.data_path = testcase_path;
        temp.polar = 0;
        temp.params = params;
        temp.mseq = mseq;
        temp.data_array = std::move( polar0 );
        push( std::move( temp ) );

        temp.data_path = testcase_path;
        temp.polar = 1;
        temp.params = params;
        temp.mseq = std::move( mseq );
        temp.data_array = std::move( polar1 );
        push( std::move( temp ) );
    }

    void readPushTwoPolarsSplitKGRS( fs::path testcase_path, size_t step )
    {
        FftParams params = IoJson::readParams( testcase_path / this->paths_.params_path );
        std::vector< int > mseq = IoJson::readMseq( testcase_path / this->paths_.mseq_path );
        auto [polar0, polar1] = IoJson::readStrobe( testcase_path / this->paths_.data_path );
        for( uint kgrs = 1; kgrs <= params.kgrs; kgrs += step )
        {
            int n1grs = -1 * (int)kgrs / 2;
            FftParams copy_params = params;
            copy_params.kgrs = kgrs;
            copy_params.n1grs = n1grs;

            FftData temp;
            temp.data_path = testcase_path;
            temp.polar = 0;
            temp.params = copy_params;
            temp.mseq = mseq;
            temp.data_array = polar0;
            push( std::move( temp ) );
    
            temp.data_path = testcase_path;
            temp.polar = 1;
            temp.params = copy_params;
            temp.mseq = mseq;
            temp.data_array = polar1;
            push( std::move( temp ) );
            std::cout << focus_str("test splitkgrs read!\n");
        }
    }

protected:
    fs::path testcases_dir_;
    ReadPathsTemplate paths_;
    std::vector< fs::path > testcases_vector_;
};

#endif // JSON_READ_DATA_QUEUE_H__