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
    json j;
    ifs >> j;
    ifs.close();
    return FftParams(j);
}

void
writeTimeToFile( const fs::path & filepath, TimeResult result )
{
    std::ofstream ofs( filepath, std::ios::trunc );
    if (!ofs.is_open()) {
        std::cerr << "TimeResult file not opened" << std::endl;
        return;
    }
    ulong funcstart = result.writeStart;
    ulong funcend = result.readEnd;
    double funclength = ((double)funcend - funcstart) / 1000000;

    ofs << "write data\n"
            << "\tstart:\t"
            << (((double)result.writeStart - funcstart) / 1000000) << " ms\n"
            << "\tend:\t"
            << (((double)result.writeEnd - funcstart) / 1000000) << " ms\n"
            << "\texecution duration:\t"
            << ((double)result.writeEnd - result.writeStart) / 1000000 << " ms\n";
    if (result.fmSignFFTStart != 0) {
        ofs << "sign fft\n"
                << "\tstart:\t"
                << (((double)result.fmSignFFTStart - funcstart) / 1000000) << " ms\n"
                << "\tend:\t"
                << (((double)result.fmSignFFTEnd - funcstart) / 1000000) << " ms\n"
                << "\texecution duration:\t"
                << ((double)result.fmSignFFTEnd - result.fmSignFFTStart) / 1000000 << " ms\n";
        ofs << "data fft\n"
                << "\tstart:\t"
                << (((double)result.fmDataFFTStart - funcstart) / 1000000) << " ms\n"
                << "\tend:\t"
                << (((double)result.fmDataFFTEnd - funcstart) / 1000000) << " ms\n"
                << "\texecution duration:\t"
                << ((double)result.fmDataFFTEnd - result.fmDataFFTStart) / 1000000 << " ms\n";
    }
    ofs << "fft\n"
            << "\tstart:\t"
            << (((double)result.FFTStart - funcstart) / 1000000) << " ms\n"
            << "\tend:\t"
            << (((double)result.FFTEnd - funcstart) / 1000000) << " ms\n"
            << "\texecution duration:\t"
            << ((double)result.FFTEnd - result.FFTStart) / 1000000 << " ms\n";
    ofs << "read data\n"
            << "\tstart:\t"
            << (((double)result.readStart - funcstart) / 1000000) << " ms\n"
            << "\tend:\t"
            << (((double)result.readEnd - funcstart) / 1000000) << " ms\n"
            << "\texecution duration:\t"
            << ((double)result.readEnd - result.readStart) / 1000000 << " ms\n";
    ofs << "FFT full execution time: " << funclength << std::endl;
}

