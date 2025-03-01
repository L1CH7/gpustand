#ifndef TYPES_H__
#define TYPES_H__

#include <string>
#include <vector>
#include <cstdint>
#include <filesystem>
namespace fs = std::filesystem;

#include <thread>

struct DeviceIdentity {
    std::string name;
    std::string version;
    std::string plat_name;
    std::string plat_version;
};

struct FftParams
{
    bool is_am;

    uint nl;
    int n1grs;
    uint kgrs;
    uint true_nihs;
    int nfgd_fu;
    uint kgd;
    uint shgd;
    uint ndec;
    uint dlstr;
    uint samples_num;

    uint log2N;

    std::vector<int> mseq;
};


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
};

#endif // TYPES_H__