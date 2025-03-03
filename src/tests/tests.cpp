#include "tests.h"

void TestTemplate2Polars( FftCreator & fft, const Paths & paths )
{
    FftParams params = readJsonParams( paths.params_path, paths.mseq_path );

    auto [ polar0, polar1 ] = readVectorFromJsonFile2Polars< std::complex< int > >( paths.input_path );
    auto polar0_ptr = reinterpret_cast< cl_int2 * >( polar0.data() );
    auto polar1_ptr = reinterpret_cast< cl_int2 * >( polar1.data() );

    // FftCreator fft( handler, params, polar0_ptr );
    fft.update( params, polar0_ptr );
    auto times0 = fft.compute();
    polar0.clear();
    auto res0_ptr = fft.getFftResult();

    fft.update( params, polar1_ptr );
    auto times1 = fft.compute();
    polar1.clear();
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
    // for( size_t i = 0, offset = 0, step = res_size / params.nl; i < params.nl; ++i, offset += step )
    // {
    //     std::string ray = "Ray" + std::to_string( i );
    //     resv0rays[i] = std::vector< std::complex< float > >( res0_complex_ptr + offset, res0_complex_ptr + offset + step );
    //     j_out["Polar0"][ray] = resv0rays[i];

    //     resv1rays[i] = std::vector< std::complex< float > >( res1_complex_ptr + offset, res1_complex_ptr + offset + step );
    //     j_out["Polar1"][ray] = resv1rays[i];
    // }
    resv0rays.clear();
    resv1rays.clear();
    std::ofstream ofs( paths.result_data_path );
    ofs << j_out.dump(4);
    // writeTimeStampsToFile( paths.result_time_path.parent_path() / "time0.out", times0 );
    // writeTimeStampsToFile( paths.result_time_path.parent_path() / "time1.out", times1 );
    writeTimeStampsToJsonFile( paths.result_time_path.parent_path() / "time0.json", times0 );
    writeTimeStampsToJsonFile( paths.result_time_path.parent_path() / "time1.json", times1 );
}

void TestTemplate1Polar( FftCreator & fft, const Paths & paths, std::string polar_json_key )
{
    FftParams params = readJsonParams( paths.params_path, paths.mseq_path );

    // Read input vector and cast it to cl_int2
    auto polar = readVectorFromJsonFile1Polar< std::complex< int > >( paths.input_path, polar_json_key );
    auto polar_ptr = reinterpret_cast< cl_int2 * >( polar.data() );

    // FftCreator fft( handler, params, polar0_ptr );
    fft.update( params, polar_ptr );
    auto times = fft.compute();
    auto res_ptr = fft.getFftResult();

    // Get output cl_float2 array ant cast it to vector
    size_t res_size = params.nl * params.kgd * params.kgrs;
    auto res_complex_ptr = reinterpret_cast< std::complex< float > * >( res_ptr );

    json j_out;
    std::vector< std::vector< std::complex< float > > > resv0rays( params.nl );
    for( size_t i = 0, offset = 0, step = res_size / params.nl; i < params.nl; ++i, offset += step )
    {
        std::stringstream key0, key1;
        resv0rays[i] = std::vector< std::complex< float > >( res_complex_ptr + offset, res_complex_ptr + offset + step );
        key0 << "Ray" << i << "Polar" << polar_json_key.back();
        j_out[key0.str()] = resv0rays[i];
    }
    // for( size_t i = 0, offset = 0, step = res_size / params.nl; i < params.nl; ++i, offset += step )
    // {
    //     std::string ray = "Ray" + std::to_string( i );
    //     resv0rays[i] = std::vector< std::complex< float > >( res_complex_ptr + offset, res_complex_ptr + offset + step );
    //     j_out["Polar0"][ray] = resv0rays[i];
    // }

    std::ofstream ofs( paths.result_data_path );
    ofs << j_out.dump(4);
    writeTimeStampsToJsonFile( paths.result_time_path.parent_path(), times );
}

void RunSingleTest( FftCreator & fft, const fs::path & test_dir )
{
    // std::mutex m_;
    fs::path result_dir;
    // {
        // std::scoped_lock l( m_ );
        std::stringstream ss;
        ss << "test:" << test_dir.c_str() << '\n';
        std::cout << ss.str();

        result_dir = test_dir      / "result";
        if( fs::exists( result_dir ) )
            fs::remove_all( result_dir );
        fs::create_directory( result_dir );
    // }

#   ifdef ENABLE_DEBUG_COMPUTATIONS
    // print all middle-events to result dir!
    changeEventsPath( result_dir );
#   endif

    Paths paths
    {
        .params_path        = test_dir      / "in_args.json",
        .input_path         = test_dir      / "out.json",
        .mseq_path          = test_dir      / "tfpMSeqSigns.json",
        .result_data_path   = result_dir    / "data.json",
        .result_time_path   = result_dir    / "time.out"
    };
    TestTemplate2Polars( fft, paths );
}

void RunAllTests( FftCreator & fft, const fs::path & testcases_dir )
{
    // if( !handler )
    // {
    //     std::cout << error_str( "No program handler created for tests\n" );
    // }

    for( auto & testcase : fs::directory_iterator{ testcases_dir } )
    {
        if( fs::is_directory( testcase ) )
        {
            RunSingleTest( fft, testcase.path() );
        }
    }
}
