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

void TestTemplate2Polars( FftCreator & fft, const Paths & paths )
{
    FftParams params = readJsonParams( paths.params_path );
    params.shgd *= params.ndec;
    if( params.is_am )
    {
        std::cout << "AM!!!\n";
        params.log2N = ( uint32_t )std::log2( params.true_nihs ) + 1;
        params.mseq = std::vector< int >();
    }
    else
    {
        std::cout << "FM!!!\n";
        params.log2N = ( uint32_t )std::ceil( std::log2( ( double )params.dlstr / params.ndec ) );
        std::ifstream ifs( paths.mseq_path );
        json j = json::parse( ifs );
        params.mseq = std::vector< int >( j );
        params.mseq.resize( 1 << params.log2N, 0 ); // mseq should be N elems, filled with zeroes
    }

    // Read input vector and cast it to cl_int2
    std::ifstream ifs( paths.input_path );
    json j = json::parse( ifs );
    std::vector< std::complex< int > > polar0( j["polar0"] ), polar1( j["polar1"] );

    auto polar0_ptr = reinterpret_cast< cl_int2 * >( polar0.data() );
    auto polar1_ptr = reinterpret_cast< cl_int2 * >( polar1.data() );

    // FftCreator fft( handler, params, polar0_ptr );
    fft.update( params, polar0_ptr );
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
    ofs << j_out.dump(4);
    writeTimeStampsToFile( paths.result_time_path.parent_path() / "time0.out", times0 );
    writeTimeStampsToFile( paths.result_time_path.parent_path() / "time1.out", times1 );
}

void RunSingleTest( FftCreator & fft, const fs::path & test_dir )
{
    std::cout << "test:" << test_dir.c_str() << '\n';

    fs::path result_dir     = test_dir      / "result";
    if( !fs::exists( result_dir ) )
        fs::create_directory( result_dir );

#   ifdef ENABLE_DEBUG_COMPUTATIONS
    // print all middle-events to result dir!
    changeEventsPath( result_dir );
#   endif

    Paths paths
    {
        .params_path        = test_dir      / "in_args.json",
        .input_path         = test_dir      / "out.json",
        .mseq_path          = test_dir      / "tfpMSeqSigns.json",
        .result_data_path   = result_dir    / "data.out",
        .result_time_path   = result_dir    / "time.out"
    };
    TestTemplate2Polars( fft, paths );
}

void RunAllTests( FftCreator & fft, const fs::path & testcases_dir )
{
    // if( !handler )
    // {
    //     std::cout << "No program handler created for tests\n"_red;
    // }

    for( auto & testcase : fs::directory_iterator{ testcases_dir } )
    {
        if( fs::is_directory( testcase ) )
        {
            RunSingleTest( fft, testcase.path() );
        }
    }
}

#endif // TESTS_H__