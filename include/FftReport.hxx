#pragma once

#include <vector>
#include <complex>
#include <filesystem>
namespace fs = std::filesystem;

#include <FftParams.hxx>
#include <TimeResult.hxx>
#include <DataHashWrapper.hxx>
#include <StrobeHash.hxx>
#include <JsonHelper.hxx>

struct FftReport
{
    fs::path report_dir;
    fs::path result_dir;
    fs::path data_path;
    size_t task_index;
    uint8_t polar;
    FftParams params;  
    std::vector< std::complex< float > > out_array;
    TimeResult time;
    DataHashWrapper src_verif;
    DataHashWrapper out_verif;
    bool accurate_result;
};

static void to_json( json & j, const FftReport & report )
{
    j["report_path"]            = report.data_path;
    j["polar"]                  = report.polar;
    j["params"]                 = report.params;
    j["time"]                   = report.time;
    
    json j_verif;
    j_verif["src_verif"]     = report.src_verif;
    j_verif["out_verif"]  = report.out_verif;
    j_verif["is_data_valid"]    = ( int )report.accurate_result;
    j["verification"]           = j_verif;
}

// static void from_json( const json & j, FftReport & report )
// {

// }
