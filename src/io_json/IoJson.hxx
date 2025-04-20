#ifndef IO_JSON_TESTS_HXX__
#define IO_JSON_TESTS_HXX__

#include <iostream>
#include <vector>
#include <fstream>

#include <error.hxx>
#include <CLDefs.hxx>

#include <JsonHelper.hxx>
#include <FftParams.hxx>
#include <FftReport.hxx>

#include <filesystem>
namespace fs = std::filesystem;

namespace IoJson
{
    FftParams
    readParams( const fs::path & file_path );

    std::vector< int >
    readMseq( const fs::path & mseq_path );

    std::pair< std::vector< std::complex< int > >, std::vector< std::complex< int > > >
    readStrobe( const fs::path & data_path );
    
    std::pair< 
        std::vector< std::complex< float > >, 
        std::vector< std::complex< float > > 
    >
    readVerificationSeq( const fs::path & ftps_path, const size_t nl );

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
    writeReport( const fs::path & file_path, const FftReport & report );
    
    void
    writeFftResult( const fs::path & file_path, std::vector< std::complex< float > > & out_array, const uint8_t polar, const FftParams & params );
};

#endif // IO_JSON_TESTS_HXX__
