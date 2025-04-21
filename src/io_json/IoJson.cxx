#include <IoJson.hxx>
#include <regex>
#include <string>

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

std::pair< size_t, size_t > parseRayPolar( const std::string & str )
{
    std::regex pattern( R"(Ray(\d+)Polar(\d))" );
    std::smatch match;
    
    if( std::regex_match( str, match, pattern ) ) 
    {
        return 
        {
            std::stoi( match[1].str() ),  // Номер луча
            std::stoi( match[2].str() )   // Номер поляризации
        };
    }
    return { 0, 0 }; // Возврат по умолчанию при ошибке
}

std::pair< 
    std::vector< std::complex< float > >, 
    std::vector< std::complex< float > > 
>
IoJson::readVerificationSeq( const fs::path & ftps_path, const size_t nl )
{
    std::ifstream ifs( ftps_path );
    if( !ifs )
    {
        std::cerr << error_str( "Ftps file not opened" ) << std::endl;
        throw;
    }
    json j = json::parse( ifs );
    ifs.close();    
    std::vector< std::vector< std::complex< float > > > polar0_m( nl ), polar1_m( nl );
    std::vector< std::complex< float > > polar0, polar1;
    for( auto & [key, val] : j.items() )
    {
        auto [ray, polar] = parseRayPolar( key );
        std::string key_s = std::string( key );
        if( polar == 0 )
            polar0_m[ ray ] = val;
        else if( polar == 1 )
            polar1_m[ ray ] = val;
    }    
    for( auto v : polar0_m ) polar0.insert( polar0.end(), v.begin(), v.end() );
    for( auto v : polar1_m ) polar1.insert( polar1.end(), v.begin(), v.end() ); 

    return std::make_pair( polar0, polar1 );
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
    json j( report );
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
