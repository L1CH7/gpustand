#ifndef IO_JSON_TESTS_H__
#define IO_JSON_TESTS_H__

#include <iostream>
#include <vector>
#include <fstream>

#include <error.h>
#include <CLDefs.h>

#include <JsonHelper.h>


FftParams
readJsonParams( const fs::path & filepath );

FftParams
readJsonParams( const fs::path & filepath, const fs::path & mseq_path );

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
std::vector< T >
readVectorFromJsonFile1Polar( const fs::path & filepath, std::string polar_json_key )
{
    std::ifstream ifs( filepath );
    json j = json::parse( ifs );
    ifs.close();
    return std::vector< T >( j[polar_json_key] );
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
writeReportToJsonFile( const fs::path & filepath, FftParams params, TimeResult t0, TimeResult t1 )

#endif // IO_JSON_TESTS_H__
