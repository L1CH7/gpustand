#include "IoJson.h"
#include <HelperFunctions.h>

FftParams
IoJson::readParams( const fs::path & file_path )
{
    std::ifstream ifs( file_path );
    if( !ifs )
    {
        std::cerr << error_str( "Params file not opened" ) << std::endl;
        return FftParams{ .nl = static_cast< uint >( -1 ) };
    }
    json j = json::parse( ifs );
    ifs.close();

    FftParams params( j );
    
    params.shgd *= params.ndec;
    if( params.is_am )
    {
        // std::cout << "AM params read!!!\n";
        params.log2N = ( uint32_t )std::log2( params.true_nihs ) + 1;
    }
    else
    {
        // std::cout << "FM params read!!!\n";
        params.log2N = ( uint32_t )std::ceil( std::log2( ( double )params.dlstr / params.ndec ) );
    }
    params.test_name = file_path.parent_path().filename().c_str();
    return std::move( params );
}

std::vector< int >
IoJson::readMseq( const fs::path & mseq_path )
{
    return readVectorFromJsonFile< int >( mseq_path );
}

std::pair< std::vector< std::complex< int > >, std::vector< std::complex< int > > >
IoJson::readStrobe( const fs::path & data_path )
{
    return readVectorFromJsonFile2Polars< std::complex< int > >( data_path );
}


std::pair< 
    std::vector< std::vector< float > >, 
    std::vector< std::vector< float > > 
>
IoJson::readVerificationSeq( const fs::path & ftps_path, const size_t N, const FftParams & params )
{
    // auto [ polar0, polar1 ] = readVectorFromJsonFile2Polars< std::complex< float > >( ftps_path ); // won`t work because there not 2 polars, but NL rays + polars
    std::ifstream ifs( ftps_path );
    if( !ifs )
    {
        std::cerr << error_str( "Ftps file not opened" ) << std::endl;
        throw;
    }
    json j = json::parse( ifs );
    ifs.close();    
    std::vector< std::complex< float > > polar0, polar1;
    for( auto & [key, val] : j.items() )
    {
        std::string key_s = std::string( key );
        if( key_s.find("Polar0") != std::string::npos )
        {
            std::vector< std::complex< float > > v( val );
            polar0.insert( polar0.end(), v.begin(), v.end() );
        }
        else if( key_s.find("Polar1") != std::string::npos )
        {
            std::vector< std::complex< float > > v( val );
            polar1.insert( polar1.end(), v.begin(), v.end() );
        }
    }    
    auto elems0 = Helper::getFirstNMaxElemsPerNl( polar0, N, params );
    auto elems1 = Helper::getFirstNMaxElemsPerNl( polar1, N, params );
    return std::make_pair( std::move( elems0 ), std::move( elems1 ) );
}

// Report = times of one polar + params
void
IoJson::writeReport( const fs::path & file_path, const FftReport & report )
{
    std::ofstream ofs( file_path, std::ios::trunc );
    if (!ofs.is_open()) {
        std::cerr << error_str( "TimeResult file not opened" ) << std::endl;
        return;
    }
    json j;
    j["report_path"]            = report.data_path;
    j["polar"]                  = report.polar;
    j["params"]                 = json( report.params );
    j["time"]                   = json( report.time );
    
    json j_verif;
    j_verif["verification"]     = report.verification;
    j_verif["result_maximums"]  = report.out_maximums;
    j_verif["is_data_valid"]    = ( int )report.is_data_valid;
    j["verification"]           = j_verif;

    ofs << j.dump( 4 );
    ofs.close();
}

void
IoJson::writeFftResult( const fs::path & file_path, std::vector< std::complex< float > > & out_array, const uint8_t polar, const FftParams & params )
{
    size_t res_size = params.nl * params.kgd * params.kgrs;
    auto res_complex_ptr = out_array.data();

    json j_out;
    std::vector< std::vector< std::complex< float > > > resv0rays( params.nl );
    for( size_t i = 0, offset = 0, step = res_size / params.nl; i < params.nl; ++i, offset += step )
    {
        std::stringstream key0, key1;
        resv0rays[i] = std::vector< std::complex< float > >( res_complex_ptr + offset, res_complex_ptr + offset + step );
        key0 << "Ray" << i << "Polar" << std::to_string( polar );
        j_out[key0.str()] = resv0rays[i];
    }

    std::ofstream ofs( file_path );
    ofs << j_out.dump(4);
    ofs.close();
}
