#ifndef JSON_HELPER_H__
#define JSON_HELPER_H__

#include <complex>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <types.h>

// Methods to convert std::complex from/to nlohmann::json
// It is necessary to write them in corresponding namespace - std
namespace std
{
    template< typename T >
    void 
    to_json( json & j, const std::complex< T > & c )
    {
        j[0] = c.real();
        j[1] = c.imag();
    }

    template< typename T >
    void 
    from_json( const json & j, std::complex< T > & c )
    {
        c = std::complex( j[0], j[1] );
    }
}

// method from_json makes possible to work struct FftParams with nlohmann/json lib
// FftParams from JSON
static void 
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

// FftParams to JSON
static void  
to_json( json & j, const FftParams & params )
{
    j["nl"]             = params.nl;
    j["kgd"]            = params.kgd;
    j["true_nihs"]      = params.true_nihs;
    j["samples_num"]    = params.samples_num;
    j["nfgd_fu"]        = params.nfgd_fu;
    j["shgd"]           = params.shgd;
    j["kgrs"]           = params.kgrs;
    j["n1grs"]          = params.n1grs;
    j["ndec"]           = params.ndec;
    j["dlstr"]          = params.dlstr;
    j["is_am"]          = static_cast< int >( params.is_am );
}

// TimeResult to JSON
static void 
to_json( json & j, const TimeResult & t )
{
    auto get_stamps = []( uint64_t _start, uint64_t _end, size_t dividor )
    {
        json stamp;
        double start, end, duration; // in milliseconds
        // double ms_inv = 1000000.;

        start = static_cast< double >( _start ) / dividor;
        end = static_cast< double >( _end ) / dividor;
        duration = end - start;

        stamp["start"] = start;
        stamp["end"] = end;
        stamp["duration"] = duration;

        return stamp;
    };

    double gpu_dividor = 1000000.; // ms
    double cpu_dividor = (double)CLOCKS_PER_SEC / 1000.; // sec->ms

    j["total"] = get_stamps( t.write_start, t.read_end, gpu_dividor );
    j["write_data"] = get_stamps( t.write_start, t.write_end, gpu_dividor );
    j["sine"] = get_stamps( t.sine_computation_start, t.sine_computation_end, gpu_dividor );

    if( t.fm_sign_fft_start )
    {
        j["fm_sign_fft"] = get_stamps( t.fm_sign_fft_start, t.fm_sign_fft_end, gpu_dividor );
        j["fm_data_fft"] = get_stamps( t.fm_data_fft_start, t.fm_data_fft_end, gpu_dividor );
    }
    j["fft"] = get_stamps( t.fft_start, t.fft_end, gpu_dividor );
    j["read_data"] = get_stamps( t.read_start, t.read_end, gpu_dividor );
    // j["cpu_start_point"] = static_cast< double >( t.cpu_start_point ) / 1000.;
    // j["cpu_end_point"] = static_cast< double >( t.cpu_end_point ) / 1000.;
    j["cpu_testing_time"] = get_stamps( t.cpu_start_point, t.cpu_end_point, cpu_dividor );
    j["date"] = t.date;
    j["time"] = t.time;
}

static void  
from_json( const json & j, StrobeResPoint & point )
{
    point = StrobeResPoint{};
    // point.
    // j[point.nl].push_back( 
    //     { 
    //         { "kgd", point.kgd },
    //         { "kgrs", point.kgrs },
    //         { "value", point.value },
    //     } 
    // );
    point.nl = j["nl"];
    point.kgd = j["kgd"];
    point.kgrs = j["kgrs"];
    point.value = j["value"];
}

static void  
to_json( json & j, const StrobeResPoint & point )
{
    // j["nl"] = point.nl;
    // j[point.nl].push_back( 
    // j["points"].push_back( 
    //     { 
    //         { "kgd", point.kgd },
    //         { "kgrs", point.kgrs },
    //         { "value", point.value },
    //     } 
    // );
    j["nl"] = point.nl;
    j["kgd"] = point.kgd;
    j["kgrs"] = point.kgrs;
    j["value"] = point.value;
}

#endif // JSON_HELPER_H__