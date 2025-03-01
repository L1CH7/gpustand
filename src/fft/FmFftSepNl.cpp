#include <GpuFourier.h>

#include <config.h>

#ifdef ENABLE_DEBUG_COMPUTATIONS
#   include <debug_computations.h>
#endif

FmFftSepNl::FmFftSepNl( std::shared_ptr< ProgramHandler > handler, const FftParams & params, cl_int2 * dataArray )  
:   FftInterface( handler, params, dataArray )
{
    std::cout<<"FmFftSepNl c-tor!\n";
}

TimeResult
FmFftSepNl::compute()
{
    std::cout<<"FmFftSepNl computing!\n";
    const int sinArrLen = 524288; //2^19
    uint32_t N = 1 << params.log2N;

    uint32_t grouplog2 = 8;
    if ((params.log2N - 1) < grouplog2)
        grouplog2 = params.log2N - 1;
    uint32_t groupSize = 1 << grouplog2;

// Print this->dataArray
#ifdef ENABLE_DEBUG_COMPUTATIONS     
    writePtrArrayToJsonFile< cl_int2, std::complex< int > >( "event_params_dataArray.json", dataArray, params.nl * params.samples_num );
#endif

    cl_command_queue_properties queue_properties = CL_QUEUE_PROFILING_ENABLE;

    cl::Buffer sinsBuffer(
        *( handler->context ), 
#ifdef ENABLE_DEBUG_COMPUTATIONS
        CL_MEM_READ_WRITE,
#else
        CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS,
#endif
        sinArrLen * sizeof( cl_float )
    );
        
    cl::Buffer inSignBuffer(
        *( handler->context ), 
#ifdef ENABLE_DEBUG_COMPUTATIONS
        CL_MEM_READ_ONLY,
#else
        CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY,
#endif
        N * sizeof( cl_int )
    );

    cl::Buffer outSignBuffer(
        *( handler->context ), 
#ifdef ENABLE_DEBUG_COMPUTATIONS
        CL_MEM_READ_WRITE,
#else
        CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS,
#endif
        N * sizeof( cl_float2 )
    );

    cl::Buffer inFFTBuffer(
        *( handler->context ), 
#ifdef ENABLE_DEBUG_COMPUTATIONS
        CL_MEM_READ_ONLY,
#else
        CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY,
#endif
        params.nl * params.samples_num * sizeof( cl_int2 )
    );

    cl::Buffer outFFTBuffer(
        *( handler->context ), 
#ifdef ENABLE_DEBUG_COMPUTATIONS
        CL_MEM_READ_WRITE,
#else
        CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS,
#endif
        params.nl * N * sizeof( cl_float2 )
    );

    cl::Buffer midIFFTBuffer(
        *( handler->context ), 
#ifdef ENABLE_DEBUG_COMPUTATIONS
        CL_MEM_READ_WRITE,
#else
        CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS,
#endif
        params.kgrs * N * sizeof( cl_float2 )
    );

    cl::Buffer outIFFTBuffer(
        *( handler->context ), 
#ifdef ENABLE_DEBUG_COMPUTATIONS
        CL_MEM_READ_WRITE,
#else
        CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY,
#endif
        params.nl * params.kgrs * params.kgd * sizeof( cl_float2 )
    );

    cl::CommandQueue queue(
        *( handler->context ), *( handler->device ), queue_properties, NULL 
    );

    cl::Event eventArray[8 + params.nl*3];
    size_t eventcounter = 0;
    cl_int error = CL_SUCCESS;

    queue.enqueueWriteBuffer(
        inSignBuffer, CL_TRUE, 0, 
        N * sizeof( cl_int ), ( const cl_int * )params.mseq.data(), 
        NULL, &eventArray[/* 0 */eventcounter++]
    );
    queue.enqueueWriteBuffer(
        inFFTBuffer, CL_TRUE, 0, 
        params.nl * params.samples_num * sizeof( cl_int2 ), dataArray, 
        NULL, &eventArray[/* 1 */eventcounter++]
    );

    auto sins_kernel_functor = cl::KernelFunctor< cl::Buffer >{ *( handler->program ), "getSinArrayTwoPi_F" };
    error = CL_SUCCESS;
    cl::EnqueueArgs sins_eargs{ queue, cl::NullRange, cl::NDRange( sinArrLen ), cl::NullRange };
    eventArray[/* 2 */eventcounter++] = sins_kernel_functor( sins_eargs, sinsBuffer, error );

// Print sinsBuffer and inFFTBuffer
#ifdef ENABLE_DEBUG_COMPUTATIONS     
    writeBufferToJsonFile< cl_float, float >( "event.sinsBuffer.json", queue, sinsBuffer, sinArrLen );
    writeBufferToJsonFile< cl_int, int >( "event.inSignBuffer.json", queue, inSignBuffer, N );
    writeBufferToJsonFile< cl_int2, std::complex< int > >( "event.inFFTBuffer.json", queue, inFFTBuffer, params.nl * params.samples_num );
#endif

    auto sign_prep_kernel_functor = cl::KernelFunctor< cl::Buffer, cl::Buffer, cl_uint, cl_uint >{ 
        *( handler->program ), "signPrep_F" 
    };
    error = CL_SUCCESS;
    cl::EnqueueArgs sign_prep_eargs{ queue, cl::NullRange, cl::NDRange( N ), cl::NullRange };
    eventArray[/* 3 */eventcounter++] = sign_prep_kernel_functor( 
        sign_prep_eargs, inSignBuffer, outSignBuffer, 
        params.log2N, 
        params.true_nihs,
        error 
    );
// Print outSignBuffer
#ifdef ENABLE_DEBUG_COMPUTATIONS     
    writeBufferToJsonFile< cl_float2, std::complex< float > >( "event.signPrep_F.outSignBuffer.json", queue, outSignBuffer, N );
#endif

    auto sign_fft_kernel_functor = cl::KernelFunctor< cl::Buffer, cl::Buffer, cl_uint, cl_uint >{ 
        *( handler->program ), "signFFT_F" 
    };
    error = CL_SUCCESS;
    cl::EnqueueArgs sign_fft_eargs{ queue, cl::NullRange, cl::NDRange( groupSize ), cl::NDRange( groupSize ) };
    eventArray[/* 4 */eventcounter++] = sign_fft_kernel_functor( 
        sign_fft_eargs, outSignBuffer, sinsBuffer, 
        params.log2N, 
        grouplog2,
        error 
    );
// Print outSignBuffer
#ifdef ENABLE_DEBUG_COMPUTATIONS     
    writeBufferToJsonFile< cl_float2, std::complex< float > >( "event.signFFT_F.outSignBuffer.json", queue, outSignBuffer, N );
#endif

    uint dlstr_ndec = params.dlstr / params.ndec;
    auto data_prep_kernel_functor = cl::KernelFunctor< cl::Buffer, cl::Buffer, cl_uint, cl_uint, cl_uint >{ 
        *( handler->program ), "dataPrepSample_F" 
    };
    error = CL_SUCCESS;
    cl::EnqueueArgs data_prep_eargs{ queue, cl::NullRange, cl::NDRange( N, params.nl ), cl::NullRange };
    eventArray[/* 5 */eventcounter++] = data_prep_kernel_functor( 
        data_prep_eargs, inFFTBuffer, outFFTBuffer, 
        params.log2N, 
        dlstr_ndec,
        params.samples_num, 
        error 
    );
// Print outFFTBuffer
#ifdef ENABLE_DEBUG_COMPUTATIONS     
    writeBufferToJsonFile< cl_float2, std::complex< float > >( "event.dataPrepSample_F.outFFTBuffer.json", queue, outFFTBuffer, params.nl * N );
#endif

    auto data_fft_kernel_functor = cl::KernelFunctor< cl::Buffer, cl::Buffer, cl_uint, cl_uint >{ 
        *( handler->program ), "dataFFT_F" 
    };
    error = CL_SUCCESS;
    cl::EnqueueArgs data_fft_eargs{ queue, cl::NullRange, cl::NDRange( groupSize, params.nl ), cl::NDRange( groupSize, 1 ) };
    eventArray[/* 6 */eventcounter++] = data_fft_kernel_functor( 
        data_fft_eargs, outFFTBuffer, sinsBuffer, 
        params.log2N, 
        grouplog2,
        error 
    );
// Print outFFTBuffer
#ifdef ENABLE_DEBUG_COMPUTATIONS     
    writeBufferToJsonFile< cl_float2, std::complex< float > >( "event.dataFFT_F.outFFTBuffer.json", queue, outFFTBuffer, params.nl * N );
#endif

    for (int nl = 0; nl < params.nl; nl++) {
        int N2nihs = N / 2 / params.true_nihs;
        auto ifft_prep_sepnl_kernel_functor = cl::KernelFunctor< cl::Buffer, cl::Buffer, cl::Buffer, cl_uint, cl_uint, cl_uint, cl_uint >{ 
            *( handler->program ), "IFFTPrepsepNl_F" 
        };
        error = CL_SUCCESS;
        cl::EnqueueArgs ifft_prep_sepnl_eargs{ queue, cl::NullRange, cl::NDRange( N, params.kgrs ), cl::NullRange };
        eventArray[eventcounter++] = ifft_prep_sepnl_kernel_functor( 
            ifft_prep_sepnl_eargs, outFFTBuffer, outSignBuffer, midIFFTBuffer,
            params.log2N, 
            params.n1grs, 
            N2nihs,
            nl,
            error 
        );
    // Print midIFFTBuffer
    #ifdef ENABLE_DEBUG_COMPUTATIONS   
    {  
        std::stringstream filename;
        filename << "event.IFFTPrepsepNl_F.[" << nl << "].midIFFTBuffer.json";
        writeBufferToJsonFile< cl_float2, std::complex< float > >( filename.str(), queue, midIFFTBuffer, params.kgrs * N );
    }
    #endif

        auto ifft_fft_sepnl_kernel_functor = cl::KernelFunctor< cl::Buffer, cl::Buffer, cl_uint, cl_uint >{ 
            *( handler->program ), "IFFT_FFTsepNl_F" 
        };
        error = CL_SUCCESS;
        cl::EnqueueArgs ifft_fft_sepnl_eargs{ queue, cl::NullRange, cl::NDRange( groupSize, params.kgrs ), cl::NDRange( groupSize, 1 ) };
        eventArray[eventcounter++] = ifft_fft_sepnl_kernel_functor( 
            ifft_fft_sepnl_eargs, midIFFTBuffer, sinsBuffer,
            params.log2N, 
            grouplog2,
            error 
        );
    // Print midIFFTBuffer
    #ifdef ENABLE_DEBUG_COMPUTATIONS  
    {
        std::stringstream filename;
        filename << "event.IFFT_FFTsepNl_F.[" << nl << "].midIFFTBuffer.json";
        writeBufferToJsonFile< cl_float2, std::complex< float > >( filename.str(), queue, midIFFTBuffer, params.kgrs * N );   
    }
    #endif

        auto ifft_post_sepnl_kernel_functor = cl::KernelFunctor< cl::Buffer, cl::Buffer, cl_uint, cl_uint, cl_uint, cl_uint, cl_uint >{ 
            *( handler->program ), "IFFTPostsepNl_F" 
        };
        error = CL_SUCCESS;
        cl::EnqueueArgs ifft_post_sepnl_eargs{ queue, cl::NullRange, cl::NDRange( params.kgd, params.kgrs ), cl::NullRange };
        eventArray[eventcounter++] = ifft_post_sepnl_kernel_functor( 
            ifft_post_sepnl_eargs, midIFFTBuffer, outIFFTBuffer,
            params.log2N, 
            params.nfgd_fu, 
            params.shgd, 
            params.ndec, 
            nl,
            error 
        );
    // Print outIFFTBuffer
    #ifdef ENABLE_DEBUG_COMPUTATIONS   
    {
        std::stringstream filename;
        filename << "event.IFFTPostsepNl_F.[" << nl << "].outIFFTBuffer.json";
        writeBufferToJsonFile< cl_float2, std::complex< float > >( filename.str(), queue, outIFFTBuffer, params.nl * params.kgrs * params.kgd );  
    }
    #endif
    }

// Print ALL outIFFTBuffer
#ifdef ENABLE_DEBUG_COMPUTATIONS   
    writeBufferToJsonFile< cl_float2, std::complex< float > >( "event.IFFTPrep_F.IFFTPostsepNl_F.json", queue, outIFFTBuffer, params.nl * params.kgrs * params.kgd );
#endif

    queue.enqueueReadBuffer(
        outIFFTBuffer, CL_TRUE, 0,
        params.nl * params.kgrs * params.kgd * sizeof( cl_float2 ), 
        outArray,
        NULL, &eventArray[eventcounter++]
    );

    queue.finish();

    int shift = 6 + params.nl * 3;
    TimeResult time = {
        .write_start            = eventArray[0].getProfilingInfo<CL_PROFILING_COMMAND_START>(),
        .write_end              = eventArray[1].getProfilingInfo<CL_PROFILING_COMMAND_END>(),
        .sine_computation_start = eventArray[2].getProfilingInfo<CL_PROFILING_COMMAND_START>(),
        .sine_computation_end   = eventArray[2].getProfilingInfo<CL_PROFILING_COMMAND_END>(),
        .fm_sign_fft_start      = eventArray[3].getProfilingInfo<CL_PROFILING_COMMAND_START>(),
        .fm_sign_fft_end        = eventArray[4].getProfilingInfo<CL_PROFILING_COMMAND_END>(),
        .fm_data_fft_start      = eventArray[5].getProfilingInfo<CL_PROFILING_COMMAND_START>(),
        .fm_data_fft_end        = eventArray[6].getProfilingInfo<CL_PROFILING_COMMAND_END>(),
        .fft_start              = eventArray[7].getProfilingInfo<CL_PROFILING_COMMAND_START>(),
        .fft_end                = eventArray[shift].getProfilingInfo<CL_PROFILING_COMMAND_END>(),
        .read_start             = eventArray[shift+1].getProfilingInfo<CL_PROFILING_COMMAND_START>(),
        .read_end               = eventArray[shift+1].getProfilingInfo<CL_PROFILING_COMMAND_END>(),
    };
    return time;
}
