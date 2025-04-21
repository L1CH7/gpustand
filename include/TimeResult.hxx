#pragma once

#include <string>
#include <cstdint>
#include <JsonHelper.hxx>

struct TimeResult
{
    uint64_t write_start;
    uint64_t write_end;

    uint64_t sine_computation_start;
    uint64_t sine_computation_end;

    uint64_t fm_sign_fft_start;
    uint64_t fm_sign_fft_end;
    uint64_t fm_data_fft_start;
    uint64_t fm_data_fft_end;

    uint64_t fft_start;
    uint64_t fft_end;

    uint64_t read_start;
    uint64_t read_end;

    uint64_t cpu_start_point;
    uint64_t cpu_end_point;
    std::string date;
    std::string time;
};

// TimeResult to JSON
static void 
to_json( json & j, const TimeResult & t )
{
    auto get_stamps = []( uint64_t _start, uint64_t _end, size_t dividor )
    {
        json stamp;
        double start, end, duration; // in milliseconds

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
    j["cpu_testing_time"] = get_stamps( t.cpu_start_point, t.cpu_end_point, cpu_dividor );
    j["date"] = t.date;
    j["time"] = t.time;
}
