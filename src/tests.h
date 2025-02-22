#ifndef TESTS_H__
#define TESTS_H__

#include <types.h>
#include <CLDefs.h>
#include <GpuFourier.h>
#include <GpuInit.h>
#include <IoJson.h>
#include <config.h>

namespace fs = std::filesystem;

struct Paths
{
    fs::path kernel;
    fs::path params_path;
    fs::path input_path;
    fs::path mseq_path;
    fs::path result_data_path;
    fs::path result_time_path;
};

void TestTemplateLanin( const Paths & paths, ProgramHandler * handler )
{
    FftParams params = readJsonParams( paths.params_path );
    params.shgd *= params.ndec;
    if( params.is_am )
    {
        params.log2N = ( uint32_t )std::log2( params.true_nihs ) + 1;
        params.mseq = std::vector< int >();
    }
    else
    {
        params.log2N = ( uint32_t )std::ceil( std::log2( ( double )params.dlstr / params.ndec ) );
        params.mseq = readVectorFromJsonFile< int >( paths.mseq_path );
    }

    // Read input vector nd cast it to cl_int2
    json j;
    std::ifstream ifs( paths.input_path );
    ifs >> j;
    std::vector< std::complex< int > > polar0( j["polar0"] ), polar1( j["polar1"] );

    auto polar0_ptr = reinterpret_cast< cl_int2 * >( polar0.data() );
    auto polar1_ptr = reinterpret_cast< cl_int2 * >( polar1.data() );

    FftCreator fft( handler, params, polar0_ptr );
    auto times0 = fft.compute();
    auto res0_ptr = fft.getFftResult();

    fft.update( params, polar1_ptr );
    auto times1 = fft.compute();
    auto res1_ptr = fft.getFftResult();


    // Get output cl_float2 array ant cast it to vector
    size_t res_size = params.nl * params.kgd * params.kgrs;
    auto res0_complex_ptr = reinterpret_cast< std::complex< float > * >( res0_ptr );
    auto res1_complex_ptr = reinterpret_cast< std::complex< float > * >( res1_ptr );

    json j_out;
    std::vector< std::vector< std::complex< float > > > resv0rays( params.nl );
    std::vector< std::vector< std::complex< float > > > resv1rays( params.nl );
    for( size_t i = 0, offset = 0, step = res_size / params.nl; i < params.nl; ++i, offset += step )
    {
        std::stringstream key0, key1;
        resv0rays[i] = std::vector< std::complex< float > >( res0_complex_ptr + offset, res0_complex_ptr + offset + step );
        key0 << "Ray" << i << "Polar0";
        j_out[key0.str()] = resv0rays[i];

        resv1rays[i] = std::vector< std::complex< float > >( res1_complex_ptr + offset, res1_complex_ptr + offset + step );
        key1 << "Ray" << i << "Polar1";
        j_out[key1.str()] = resv1rays[i];
    }

    std::ofstream ofs( paths.result_data_path );
    ofs << j_out;
    writeTimeToFile( paths.result_time_path.parent_path() / "time0.out", times0 );
    writeTimeToFile( paths.result_time_path.parent_path() / "time1.out", times1 );
}

void RunTestsSingleThread( ProgramHandler * handler )
{
    fs::path root( WORKSPACE );
    {
        Paths paths
        {
            .kernel = root / "src/fft/GpuKernels.cl",
            .params_path = "",
            .input_path = "",
            .mseq_path = "",
            .result_data_path = "",
            .result_time_path = ""
        };

        auto identity = initGpuModule( handler, paths.kernel.c_str() );

        fs::path testcases_am = root / "testcases/AM/";

        for( auto & testcase : fs::directory_iterator{ testcases_am } )
        {
            if( fs::is_directory( testcase ) )
            {
                // std::cout << testcase << '\n';
                fs::path test_path      = testcase.path();
                fs::path result_dir     = test_path     / "result";
                paths.params_path       = test_path     / "in_args.json";
                paths.input_path        = test_path     / "out.json";
                paths.mseq_path         = test_path     / "tfpMSeqSigns.json";
                paths.result_data_path  = result_dir    / "data.out";
                paths.result_time_path  = result_dir    / "time.out";
                if( !fs::exists( result_dir ) )
                {
                    fs::create_directory( result_dir );
                }
                TestTemplateLanin( paths, handler );
            }
        }
    }
}

#endif // TESTS_H__