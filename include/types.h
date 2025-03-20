#ifndef TYPES_H__
#define TYPES_H__

#include <string>
using std::literals::string_literals::operator""s;

#include <vector>
#include <complex>
#include <cstdint>
#include <thread>

#include <filesystem>
namespace fs = std::filesystem;



struct DeviceIdentity {
    std::string device_name;
    std::string device_version;
    std::string platform_name;
    std::string platform_version;
};

struct FftParams
{
    bool is_am;

    uint nl;
    int n1grs;
    uint kgrs;
    uint true_nihs;
    int nfgd_fu;
    uint kgd;
    uint shgd;
    uint ndec;
    uint dlstr;
    uint samples_num;

    uint log2N;
    // std::vector<int> mseq;
    // FftParams() = default;
    // FftParams( const FftParams & ) = default;
    // FftParams( FftParams && ) = delete;
    // FftParams & operator=( const FftParams & ) = default;
    // FftParams & operator=( FftParams && ) = delete;
    std::string test_name;
};

struct FftData
{
    fs::path data_path;
    uint8_t polar;
    FftParams params;
    std::vector< int > mseq;
    std::vector< std::complex< int > > data_array;

    FftData() = default;
    FftData( const FftData & other ) = default;
    FftData( FftData && other )
    :   data_path( other.data_path ),
        polar( other.polar ),
        params( other.params ),
        mseq( std::move( other.mseq ) ),
        data_array( std::move( other.data_array ) )
    {
    }

    FftData( fs::path data_path, uint8_t polar, FftParams params, std::vector< int > & mseq, std::vector< std::complex< int > > & data_array )
    :   data_path( data_path ),
        polar( polar ),
        params( params ),
        mseq( std::move( mseq ) ),
        data_array( std::move( data_array ) )
    {
    }

    FftData & operator=( const FftData & other ) = default;
    FftData & operator=( FftData && other )
    {
        // FftData temp{ std::move( other ) };
        // return temp;
        this->data_path = other.data_path;
        this->params = other.params;
        this->polar = other.polar;
        this->mseq = std::move( other.mseq );
        this->data_array = std::move( other.data_array );
        return *this;
    }
};

struct TimeResult
{
    uint64_t write_start;
    uint64_t write_end;

    uint64_t sine_computation_start;
    uint64_t sine_computation_end;

    uint64_t fm_sign_fft_start;
    uint64_t fm_sign_fft_end;
    uint64_t fm_data_fft_start;
    uint64_t fm_data_fft_end;

    uint64_t fft_start;
    uint64_t fft_end;

    uint64_t read_start;
    uint64_t read_end;

    uint64_t cpu_start_point;
    uint64_t cpu_end_point;
};

struct FftReport
{
    fs::path report_dir;
    fs::path result_dir;
    size_t task_index;
    fs::path data_path;
    std::vector< std::complex< float > > out_array;
    uint8_t polar;
    FftParams params;  
    TimeResult time;
};

#endif // TYPES_H__