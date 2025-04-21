#pragma once

#include <string>
#include <cstdint>
#include <JsonHelper.hxx>

struct FftParams
{
    uint is_am; // bool that represented in <0,1> not handling by JSON

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
    std::string test_name;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE( FftParams, nl, kgd, true_nihs, samples_num, nfgd_fu, shgd, kgrs, n1grs, ndec, dlstr, is_am );
};
