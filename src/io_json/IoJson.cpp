#include "IoJson.h"

// method from_json makes possible to work struct FftParams with nlohmann/json lib
void 
from_json( const json & j, FftParams & params )
{
    params.nl           = j["nl"];
    params.kgd          = j["kgd"];
    params.true_nihs    = j["true_nihs"];
    params.samples_num  = j["samples_num"];
    params.nfgd_fu      = j["nfgd_fu"];
    params.shgd         = j["shgd"];
    params.kgrs         = j["kgrs"];
    params.n1grs        = j["n1grs"];
    params.ndec         = j["ndec"];
    params.dlstr        = j["dlstr"];
    params.is_am        = static_cast< int >( j["is_am"] );
}

FftParams
readJsonParams( const fs::path & filepath )
{
    std::ifstream ifs( filepath );
    if( !ifs )
    {
        std::cerr << "Param file not opened"_red << std::endl;
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
        std::cerr << "Param file not opened"_red << std::endl;
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
writeTimeStampsToFile( const fs::path & filepath, TimeResult t )
{
    std::ofstream ofs( filepath, std::ios::trunc );
    if (!ofs.is_open()) {
        std::cerr << "TimeResult file not opened"_red << std::endl;
        return;
    }
    char * line = new char[100];
    double start, end, duration; // in milliseconds
    auto get_stamps = [ &start, &end, &duration ]( uint64_t _start, uint64_t _end )
    {
        double ms = 1. / 1000000.;
        start = static_cast< double >( _start ) * ms;
        end = static_cast< double >( _end ) * ms;
        duration = end - start;
    };

    auto print_stamps = [ &start, &end, &duration, &get_stamps, &line, &ofs ]( const char * _stamp, uint64_t _start, uint64_t _end )
    {
        get_stamps( _start, _end );
        std::sprintf( line, "%-20s%-20.5f%-20.5f%-20.5f\n", _stamp, start, end, duration );
        ofs << line;
    };

    ofs << "****" << ( t.fm_sign_fft_start ? "FM" : "AM" ) << "_FFT****\n\n";

    get_stamps( t.write_start, t.read_end );
    std::sprintf( line, "%-20s%-20.5f\n", "Start:", start );
    ofs << line;

    std::sprintf( line, "%-20s%-20.5f\n", "End:", end );
    ofs << line;

    std::sprintf( line, "%-20s%-20.5f\n\n", "Duration:", end - start );
    ofs << line;

    std::sprintf( line, "%-20s%-20s%-20s%-20s\n", "Stamp", "Start", "End", "Duration" );
    ofs << line;

    print_stamps( "Write data", t.write_start, t.write_end );
    print_stamps( "Sine", t.sine_computation_start, t.sine_computation_end );
    if( t.fm_sign_fft_start )
    {
        print_stamps( "FM Sign FFT", t.fm_sign_fft_start, t.fm_sign_fft_end );
        print_stamps( "FM Data FFT", t.fm_data_fft_start, t.fm_data_fft_end );
    }
    print_stamps( "FFT", t.fft_start, t.fft_end );
    print_stamps( "Read data", t.read_start, t.read_end );

    delete[] line;
    ofs.close();
}

void
writeTimeStampsToJsonFile( const fs::path & filepath, TimeResult t )
{
    std::ofstream ofs( filepath, std::ios::trunc );
    if (!ofs.is_open()) {
        std::cerr << "TimeResult file not opened"_red << std::endl;
        return;
    }
    auto get_stamps = []( uint64_t _start, uint64_t _end )
    {
        json stamp;
        double start, end, duration; // in milliseconds
        double ms = 1. / 1000000.;

        start = static_cast< double >( _start ) * ms;
        end = static_cast< double >( _end ) * ms;
        duration = end - start;

        stamp["start"] = start;
        stamp["end"] = end;
        stamp["duration"] = duration;

        return stamp;
    };

    json j, stamp;

    j["total"] = get_stamps( t.write_start, t.read_end );
    j["write_data"] = get_stamps( t.write_start, t.write_end );
    j["sine"] = get_stamps( t.sine_computation_start, t.sine_computation_end );

    if( t.fm_sign_fft_start )
    {
        j["fm_sign_fft"] = get_stamps( t.fm_sign_fft_start, t.fm_sign_fft_end );
        j["fm_data_fft"] = get_stamps( t.fm_data_fft_start, t.fm_data_fft_end );
    }
    j["fft"] = get_stamps( t.fft_start, t.fft_end );
    j["read_data"] = get_stamps( t.read_start, t.read_end );

    ofs << j.dump(4);
    ofs.close();
}

