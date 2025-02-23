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
    params.is_am        = (int)j["is_am"];
}

FftParams
readJsonParams( const fs::path & filepath )
{
    std::ifstream ifs( filepath );
    FftParams params;
    params.nl = -1;
    if( !ifs )
    {
        std::cerr << "Param file not opened" << std::endl;
        return params;
    }
    json j = json::parse( ifs );
    ifs.close();
    return FftParams(j);
}

void
writeTimeStampsToFile( const fs::path & filepath, TimeResult t )
{
    std::ofstream ofs( filepath, std::ios::trunc );
    if (!ofs.is_open()) {
        std::cerr << "TimeResult file not opened" << std::endl;
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

