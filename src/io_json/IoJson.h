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
    if( !ifs )
    {
        std::cerr << error_str( "Data polarisation file not opened" ) << std::endl;
        throw;
    }
    json j = json::parse( ifs );
    ifs.close();
    std::vector< T > polar0( j["polar0"] ), polar1( j["polar1"] );
    return std::move( std::make_pair( polar0, polar1 ) );
}

void
writeTimeStampsToFile( const fs::path &, const TimeResult & );

void
writeReportToJsonFile( const fs::path & filepath, const std::string & report_path, const uint8_t polar, const FftParams & params, const TimeResult & t );

#endif // IO_JSON_TESTS_H__
