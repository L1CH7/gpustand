#ifndef TESTS_HXX__
#define TESTS_HXX__

#include <CLDefs.hxx>
#include <GpuFourier.hxx>
#include <GpuInit.hxx>
#include <config.hxx>
#include <error.hxx>

struct Paths
{
    fs::path params_path;
    fs::path input_path;
    fs::path mseq_path;
    fs::path result_data_path;
    fs::path result_time_path;
};

// void TestTemplate1Polar( FftCreator & fft, const Paths & paths, std::string polar_json_key );

// void TestTemplate2Polars( FftCreator & fft, const Paths & paths );

// void RunSingleTest( std::shared_ptr< ProgramHandler > handler, const fs::path & test_dir );

void testExecutionPool( std::shared_ptr< ProgramHandler > handler, fs::path root_dir, size_t read_thread_num = 1, size_t computing_thread_num = 1, size_t write_thread_num = 1 );

void RunAllTests( std::shared_ptr< ProgramHandler > handler, fs::path root_dir, size_t read_thread_num = 1, size_t computing_thread_num = 1, size_t write_thread_num = 1 );

void RunTestIterateKGD();

void RunSomeTests();

#endif // TESTS_HXX__