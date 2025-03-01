#ifndef TESTS_H__
#define TESTS_H__

#include <types.h>
#include <CLDefs.h>
#include <GpuFourier.h>
#include <GpuInit.h>
#include <IoJson.h>
#include <config.h>

#ifdef ENABLE_DEBUG_COMPUTATIONS
#   include <debug_computations.h> 
    // std::unique_ptr< fs::path > events_path = nullptr;
#endif

struct Paths
{
    fs::path params_path;
    fs::path input_path;
    fs::path mseq_path;
    fs::path result_data_path;
    fs::path result_time_path;
};

void TestTemplate1Polar( FftCreator & fft, const Paths & paths, std::string polar_json_key );

void TestTemplate2Polars( FftCreator & fft, const Paths & paths );

void RunSingleTest( FftCreator & fft, const fs::path & test_dir );

void RunAllTests( FftCreator & fft, const fs::path & testcases_dir );

#endif // TESTS_H__