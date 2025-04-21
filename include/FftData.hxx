#pragma once

#include <vector>
#include <complex>
#include <filesystem>
namespace fs = std::filesystem;

#include <FftParams.hxx>
#include <DataHashWrapper.hxx>
#include <StrobeHash.hxx>

struct FftData
{
    fs::path data_path;
    uint8_t polar;
    FftParams params;
    std::vector< int > mseq;
    std::vector< std::complex< int > > data_array;
    DataHashWrapper verification;

    FftData() = default;
    FftData( const FftData & other ) = default;
    FftData( FftData && other )
    :   data_path( other.data_path ),
        polar( other.polar ),
        params( other.params ),
        mseq( std::move( other.mseq ) ),
        data_array( std::move( other.data_array ) ),
        verification( std::move( other.verification ) )
    {
    }

    FftData & operator=( const FftData & other ) = default;
    FftData & operator=( FftData && other )
    {
        this->data_path = other.data_path;
        this->params = other.params;
        this->polar = other.polar;
        this->mseq = std::move( other.mseq );
        this->data_array = std::move( other.data_array );
        this->verification = std::move( other.verification );
        return *this;
    }
};
