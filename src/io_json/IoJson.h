#ifndef IO_JSON_TESTS_H__
#define IO_JSON_TESTS_H__

#include <iostream>
#include <vector>
#include <fstream>

#include <error.h>
#include <CLDefs.h>

#include <JsonHelper.h>

namespace IoJson
{
    FftParams
    readParams( const fs::path & file_path );

    std::vector< int >
    readMseq( const fs::path & mseq_path );

    std::pair< std::vector< std::complex< int > >, std::vector< std::complex< int > > >
    readStrobe( const fs::path & data_path );

    template< typename T >
    std::vector< T >
    readVectorFromJsonFile( const fs::path & file_path )
    {
        std::ifstream ifs( file_path );
        json j = json::parse( ifs );
        ifs.close();
        return std::vector< T >( j );
    }

    template< typename T >
    std::pair< std::vector< T >, std::vector< T > >
    readVectorFromJsonFile2Polars( const fs::path & file_path )
    {
        std::ifstream ifs( file_path );
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
    writeReport( const fs::path & file_path, const std::string & report_path, const uint8_t polar, const FftParams & params, const TimeResult & t );

    void
    writeFftResult( const fs::path & file_path, std::vector< std::complex< float > > & out_array, const uint8_t polar, const FftParams & params );
};

#endif // IO_JSON_TESTS_H__
