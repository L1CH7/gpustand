#include "IoJson.h"

FftParams
readJsonParams( const fs::path & filepath )
{
    std::ifstream ifs( filepath );
    if( !ifs )
    {
        std::cerr << error_str( "Param file not opened" ) << std::endl;
        return FftParams{ .nl = static_cast< uint >( -1 ) };
    }
    json j = json::parse( ifs );
    ifs.close();

    FftParams params(j);
    
    params.shgd *= params.ndec;
    if( params.is_am )
    {
        std::cout << "AM params read!!!\n";
        params.log2N = ( uint32_t )std::log2( params.true_nihs ) + 1;
    }
    else
    {
        std::cout << "FM params read!!!\n";
        params.log2N = ( uint32_t )std::ceil( std::log2( ( double )params.dlstr / params.ndec ) );
    }
    return std::move( params );
}

FftParams
readJsonParams( const fs::path & filepath, const fs::path & mseq_path )
{
    std::ifstream ifs( filepath );
    if( !ifs )
    {
        std::cerr << error_str( "Param file not opened" ) << std::endl;
        return FftParams{ .nl = static_cast< uint >( -1 ) };
    }
    json j = json::parse( ifs );
    ifs.close();

    FftParams params(j);
    
    params.shgd *= params.ndec;
    if( params.is_am )
    {
        std::cout << "AM params read!!!\n";
        params.log2N = ( uint32_t )std::log2( params.true_nihs ) + 1;
        // params.mseq = std::vector< int >();
    }
    else
    {
        std::cout << "FM params read!!!\n";
        params.log2N = ( uint32_t )std::ceil( std::log2( ( double )params.dlstr / params.ndec ) );
        // params.mseq = readVectorFromJsonFile< cl_int >( mseq_path );
        // params.mseq.resize( 1 << params.log2N, 0 ); // mseq should be N elems, filled with zeroes
    }
    return std::move( params );
}

void
writeTimeStampsToJsonFile( const fs::path & filepath, const TimeResult & t )
{
    std::ofstream ofs( filepath, std::ios::trunc );
    if (!ofs.is_open()) {
        std::cerr << error_str( "TimeResult file not opened" ) << std::endl;
        return;
    }
    json j( t );
    ofs << j.dump(4);
    ofs.close();
}

// Report = times of each polars + params
void
writeReportToJsonFile( const fs::path & filepath, const std::string & report_path, const uint8_t polar, const FftParams & params, const TimeResult & t )
{
    std::ofstream ofs( filepath, std::ios::trunc );
    if (!ofs.is_open()) {
        std::cerr << error_str( "TimeResult file not opened" ) << std::endl;
        return;
    }
    json j;
    j["report_path"]    = report_path;
    j["polar"]          = polar;
    j["params"]         = json( params );
    j["time"]           = json( t );
    // j["time"]["polar0"] = json(t0);
    // j["time"]["polar1"] = json(t1);
    ofs << j.dump( 4 );
    ofs.close();
}

void
writeFftResultToJsonFile( const fs::path & filepath, std::vector< std::complex< float > > & out_array, const uint8_t polar, const FftParams & params )
{
    size_t res_size = params.nl * params.kgd * params.kgrs;
    // auto res_complex_ptr = reinterpret_cast< std::complex< float > * >( out_array );
    auto res_complex_ptr = out_array.data();

    json j_out;
    std::vector< std::vector< std::complex< float > > > resv0rays( params.nl );
    for( size_t i = 0, offset = 0, step = res_size / params.nl; i < params.nl; ++i, offset += step )
    {
        std::stringstream key0, key1;
        resv0rays[i] = std::vector< std::complex< float > >( res_complex_ptr + offset, res_complex_ptr + offset + step );
        // resv0rays[i] = std::vector< std::complex< float > >( out_array.begin() + offset, out_array.begin() + offset + step );
        key0 << "Ray" << i << "Polar" << std::to_string( polar );
        j_out[key0.str()] = resv0rays[i];
    }

    std::ofstream ofs( filepath );
    ofs << j_out.dump(4);
    ofs.close();
}
