#ifndef TYPES_H__
#define TYPES_H__

#include <string>
#include <vector>

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
    unsigned long writeStart;
    unsigned long writeEnd;

    unsigned long sineComputationStart;
    unsigned long sineComputationEnd;

    unsigned long fmSignFFTStart;
    unsigned long fmSignFFTEnd;
    unsigned long fmDataFFTStart;
    unsigned long fmDataFFTEnd;

    unsigned long FFTStart;
    unsigned long FFTEnd;

    unsigned long readStart;
    unsigned long readEnd;
};

#endif // TYPES_H__