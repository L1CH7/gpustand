#include "IoJson.h"

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

    FftParams params(j);
    
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

// Report = times of one polar + params
void
IoJson::writeReport( const fs::path & file_path, const std::string & report_path, const uint8_t polar, const FftParams & params, const TimeResult & t )
{
    std::ofstream ofs( file_path, std::ios::trunc );
    if (!ofs.is_open()) {
        std::cerr << error_str( "TimeResult file not opened" ) << std::endl;
        return;
    }
    json j;
    j["report_path"]    = report_path;
    j["polar"]          = polar;
    j["params"]         = json( params );
    j["time"]           = json( t );
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
