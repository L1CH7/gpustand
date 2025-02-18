#include <types.h>
#include <CLDefs.h>
#include <GpuFourier.h>
#include <ReadWriteFunctions.h>
#include <GpuInit.h>
#include <config.h> 

void AmTest()
{
    size_t platform_id = 3;
    size_t device_id = 0;
    std::string error;
    ProgramHandler * handler = makeProgramHandler( platform_id, device_id, error );

    std::string kernel_file = "src/fft/GpuKernels.cl";
    auto identity = initGpuModule( handler, kernel_file );

    FftParams params = readJsonParams( IN_ARGS );
    params.shgd *= params.ndec;
    params.is_am = true;
    params.log2N = params.is_am 
        ? (uint32_t)std::log2(params.true_nihs) + 1 
        : (uint32_t)std::ceil(std::log2((double)params.dlstr/params.ndec));

    // Read input vector nd cast it to cl_int2
    auto input = readVectorFromJsonFile< std::complex< int > >( DATA_FILE );
    auto input_ptr = reinterpret_cast< cl_int2 * >( input.data() );

    FftCreator fft( handler, params, input_ptr );
    auto times = fft.compute();

    auto res_ptr = fft.getFftResult();

    // Get output cl_float2 array ant cast it to vector
    auto res_complex_ptr = reinterpret_cast< std::complex< float > * >( res_ptr );
    size_t res_size = params.nl * params.kgd * params.kgrs;
    std::vector< std::complex< float > > resv( res_complex_ptr, res_complex_ptr + res_size );

    writeVectorToJsonFile( RESULT_FILE, resv );
    writeTimeToFile( RESULT_TIME, times, 0 );
}

int main()
{
    // Setup red coloring of cerr
    // std::cerr << "\033[0;31m" << std::endl;

    // std::cout << "std::complex<int>=" << sizeof(std::complex<int>) << '\n';
    // std::cout << "cl_int2=" << sizeof(cl_int2) << '\n';
    // std::cout << "std::complex<float>=" << sizeof(std::complex<float>) << '\n';
    // std::cout << "cl_float2=" << sizeof(cl_float2) << '\n';

    // float re=63, im=74261;
    // std::complex<float> a(re, im); 
    // auto ptr_a = &a;
    // cl_float2 cl_a;
    // cl_a.s0 = re;
    // cl_a.s1 = im;
    // auto ptr_cl_a = &cl_a;
    // ptr_a = reinterpret_cast<std::complex<float>*>(ptr_cl_a);
    // std::cout << "re=" << ptr_a->real() << " im=" << ptr_a->imag() << '\n';
    // cl_float2 * bar = reinterpret_cast<cl_float2*>(ptr_a);
    // std::cout << "re=" << bar->s0 << " im=" << bar->s1 << '\n';

    // std::complex<float> foo{bar->s0, bar->s1};
    // assert(foo == *ptr_a);


    AmTest();
    return 0;
}