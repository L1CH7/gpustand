#ifndef READ_WRITE_FUNCTIONS_H__
#define READ_WRITE_FUNCTIONS_H__

#include <iostream>
#include <vector>
#include <complex>

#include <GpuFourier.h>
#include <types.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

FftParams
readJsonParams(const std::string &filename);

// Methods to convert std::complex from/to nlohmann::json
// It is necessary to write them in corresponding namespace - std
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

template< typename T >
std::vector< T >
readVectorFromJsonFile( const std::string & filename )
{
    json j;
    std::ifstream ifs( filename );
    ifs >> j;
    return std::vector< T >( j );
}

void
writeTimeToFile(
    std::string fileName,
    TimeResult result);

#endif // READ_WRITE_FUNCTIONS_H__
