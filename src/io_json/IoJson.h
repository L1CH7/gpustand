#ifndef IO_JSON_TESTS_H__
#define IO_JSON_TESTS_H__

#include <iostream>
#include <vector>
#include <complex>
#include <fstream>
#include <filesystem>
namespace fs = std::filesystem;

#include <types.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

FftParams
readJsonParams( const fs::path & filepath );

// Methods to convert std::complex from/to nlohmann::json
// It is necessary to write them in corresponding namespace - std
namespace std
{
    template< typename T >
    void 
    to_json( json & j, const std::complex< T > & c )
    {
        // j = json{ [ c.real(), c.imag() ] };
        j[0] = c.real();
        j[1] = c.imag();
    }

    template< typename T >
    void 
    from_json( const json & j, std::complex< T > & c )
    {
        c = std::complex( j[0], j[1] );
    }
}

template< typename T >
void 
writeVectorToJsonFile( const fs::path & filepath, const std::vector< T > & arr )
{
    json j( arr );
    std::ofstream ofs( filepath );
    ofs << j.dump( 4 );
    ofs.close();
}

template< typename T >
std::vector< T >
readVectorFromJsonFile( const fs::path & filepath )
{
    std::ifstream ifs( filepath );
    json j = json::parse( ifs );
    ifs.close();
    return std::vector< T >( j["polar0"] );
}

template< typename T >
std::pair< std::vector< T >, std::vector< T > >
readVectorFromJsonFile_2Polars( const fs::path & filepath )
{
    std::ifstream ifs( filepath );
    json j = json::parse( ifs );
    ifs.close();
    std::vector< T > polar0( j["polar0"] ), polar1( j["polar1"] );

    return std::make_pair( polar0, polar1 );
}

void
writeTimeToFile( const fs::path & filepath, TimeResult result );

#endif // IO_JSON_TESTS_H__
