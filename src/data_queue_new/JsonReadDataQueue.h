#ifndef JSON_READ_DATA_QUEUE_H__
#define JSON_READ_DATA_QUEUE_H__

#include <ReadDataQueueInterface.h>
#include <types.h>
#include <IoJson.h>
#include <error.h>
struct PathsTemplate
{
    fs::path params_path;
    fs::path mseq_path;
    fs::path data_path;
};

class JsonReadDataQueue : public ReadDataQueueInterface< FftData >
{
public:
    JsonReadDataQueue() = delete;

    JsonReadDataQueue( const size_t num_threads, const fs::path & testcases_dir, PathsTemplate paths )
    :   ReadDataQueueInterface< FftData >( num_threads ),
        // testcases_dir_( testcases_dir ),
        // dir_it_( fs::directory_iterator{ testcases_dir_ } ),
        paths_( paths )
    {
        if( !fs::exists( testcases_dir ) || !fs::is_directory( testcases_dir ) )
            throw std::runtime_error( "testcases_dir not exists or not directory" );

        testcases_dir_ = testcases_dir;
        dir_it_ = fs::directory_iterator{ testcases_dir_ };
    }

    // read 2 polars, needs to be override
    std::optional< std::pair< FftData, FftData > > readData() override
    {
        if( dir_it_ == fs::directory_iterator{} )
        {
            // stop because data ended
            this->stop();
            return std::nullopt; 
        }

        fs::path testcase = *dir_it_;
        {
            std::scoped_lock lock( this->queue_mutex_ );
            ++dir_it_;
        }
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

std::cout << focus_str("read:"s + testcase.native()) << '\n';

        return std::make_pair( std::move( data0 ), std::move( data1 ) );
    }

protected:
    fs::path testcases_dir_;
    fs::directory_iterator dir_it_;
    PathsTemplate paths_;
};

#endif // JSON_READ_DATA_QUEUE_H__