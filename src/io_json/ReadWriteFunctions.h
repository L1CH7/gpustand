#ifndef READWRITEFUNCTIONS_H
#define READWRITEFUNCTIONS_H

#include <iostream>
#include <vector>
#include <complex>

#include <GpuFourier.h>
#include <types.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

FftParams
readJsonParams(const std::string &filename);


namespace std
{
    template< typename T >
    void 
    to_json( json & j, const std::complex< T > & c )
    {
        j = json{ { "real", c.real() }, { "imag", c.imag() } };
    }

    template< typename T >
    void 
    from_json( const json & j, std::complex< T > & c )
    {
        c = std::complex( j["real"], j["imag"] );
    }
}


template< typename T >
void 
writeVectorToJsonFile( const std::string & filename, const std::vector< T > & arr )
{
    json j(arr);
    std::ofstream ofs( filename );
    ofs << j.dump(4);
}

// #define PREPARE_DATA

template< typename T >
std::vector< T >
#ifdef PREPARE_DATA
readVectorFromJsonFile( const std::string & filename, size_t kgd, size_t samples_num )
#else
readVectorFromJsonFile( const std::string & filename )
#endif
{
    json j;
    std::ifstream ifs( filename );
    // std::cout << __PRETTY_FUNCTION__ << "DEBUG\n";
    ifs >> j;
#ifdef PREPARE_DATA  
    std::vector< T >    v_in( j ),
                        v_out( samples_num );
    // std::vector< T >::iterator 
    auto in_it = v_in.begin(), out_it = v_out.begin();
    bool data_end_flag = false; // true if reached end of source data. elements in out data must be zeroes
    for(; out_it != v_out.end(); ++out_it )
    {
        if( data_end_flag )
            *out_it = 0;
        else
            for( size_t d = 0; d < kgd && !data_end_flag; ++d, ++in_it )
            {
                if( in_it != v_in.end() )
                    *out_it += *in_it;
                else
                    data_end_flag = true;
            }
    }
    return v_out;
#else
    return std::vector< T >( j );
#endif
}



std::vector<int>
readTfpMSeqSignsFile(
    std::string fileName);

void
writeResultToUniteFile(
    std::string fileName,
    int polar,
    std::complex<float>* mas,
    int nl,
    int kgd,
    int kgrs);

void
writeResultToUniteFile(
    std::string fileName,
    int polar,
    cl_float2* mas,
    int nl,
    int kgd,
    int kgrs);

void
writeTimeToFile(
    std::string fileName,
    TimeResult result,
    int polarNumber);

std::vector<std::complex<int>>
readDataFile(std::string fileName);

FftParams
readInParams(const std::string &filename);

#endif // READWRITEFUNCTIONS_H

