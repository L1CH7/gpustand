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
    return FftParams(j);
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
        params.mseq = std::vector< int >();
    }
    else
    {
        std::cout << "FM params read!!!\n";
        params.log2N = ( uint32_t )std::ceil( std::log2( ( double )params.dlstr / params.ndec ) );
        params.mseq = readVectorFromJsonFile< cl_int >( mseq_path );
        params.mseq.resize( 1 << params.log2N, 0 ); // mseq should be N elems, filled with zeroes
    }
    return std::move( params );
}

void
writeTimeStampsToJsonFile( const fs::path & filepath, TimeResult t )
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
writeReportToJsonFile( const fs::path & filepath, FftParams params, TimeResult t0, TimeResult t1 )
{
    std::ofstream ofs( filepath, std::ios::trunc );
    if (!ofs.is_open()) {
        std::cerr << error_str( "TimeResult file not opened" ) << std::endl;
        return;
    }
    json j;
    j["params"] = json(params);
    j["time"]["polar0"] = json(t0);
    j["time"]["polar1"] = json(t1);
    ofs << j.dump(4);
    ofs.close();
}
