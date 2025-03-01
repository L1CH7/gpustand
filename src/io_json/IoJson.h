#ifndef IO_JSON_TESTS_H__
#define IO_JSON_TESTS_H__

#include <iostream>
#include <vector>
#include <complex>
#include <fstream>

#include <types.h>
#include <error.h>

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
    return std::vector< T >( j );
}

template< typename T >
std::pair< std::vector< T >, std::vector< T > >
readVectorFromJsonFile2Polars( const fs::path & filepath )
{
    std::ifstream ifs( filepath );
    json j = json::parse( ifs );
    ifs.close();
    std::vector< T > polar0( j["polar0"] ), polar1( j["polar1"] );
    return std::move(std::make_pair( std::move(polar0), std::move(polar1) ));

}

void
writeTimeStampsToFile( const fs::path &, TimeResult );

void
writeTimeStampsToJsonFile( const fs::path &, TimeResult );

#endif // IO_JSON_TESTS_H__
