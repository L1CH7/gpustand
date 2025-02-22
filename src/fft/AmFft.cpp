#include <GpuFourier.h>

#include <config.h>

#ifdef ENABLE_DEBUG_COMPUTATIONS
#   include <debug_computations.h>
#endif

AmFft::AmFft( ProgramHandler * handler, const FftParams & params, cl_int2 * dataArray )  
:   FftInterface( handler, params, dataArray )
{
    std::cout<<"AmFft!\n";
}

TimeResult
AmFft::compute()
{
    const uint32_t sinArrLen = 1 << 19; // 2^19 = 524288
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
        

    cl::Buffer inFFTBuffer(
        *( handler->context ), 
#ifdef ENABLE_DEBUG_COMPUTATIONS
        CL_MEM_READ_ONLY,
#else
        CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY,
#endif
        params.nl * params.samples_num * sizeof( cl_int2 )
    );


    cl::Buffer midFFTBuffer(
        *( handler->context ), 
#ifdef ENABLE_DEBUG_COMPUTATIONS
        CL_MEM_READ_WRITE,
#else
        CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS,
#endif
          params.nl * params.kgd * N * sizeof( cl_float2 )
    );


    cl::Buffer outFFTBuffer(
        *( handler->context ), CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY,
        params.nl * params.kgrs * params.kgd * sizeof( cl_float2 )
    );


    cl::CommandQueue queue(
        *( handler->context ), *( handler->device ), queue_properties, NULL 
    );

    cl::Event eventArray[6];
    cl_int error = CL_SUCCESS;

    queue.enqueueWriteBuffer(
        inFFTBuffer, CL_TRUE, 0,
        params.nl * params.samples_num * sizeof(cl_int2),
        dataArray, NULL, &eventArray[0]
    );

    auto sins_kernel_functor = cl::KernelFunctor< cl::Buffer >{ *( handler->program ), "getSinArrayTwoPi_F" };
    error = CL_SUCCESS;
    cl::EnqueueArgs sins_eargs{ queue, cl::NullRange, cl::NDRange( sinArrLen ), cl::NullRange };
    eventArray[1] = sins_kernel_functor( sins_eargs, sinsBuffer, error );

// Print sinsBuffer and inFFTBuffer
#ifdef ENABLE_DEBUG_COMPUTATIONS     
    writeBufferToJsonFile< cl_float, float >( "event.sinsBuffer.json", queue, sinsBuffer, sinArrLen );
    writeBufferToJsonFile< cl_int2, std::complex< int > >( "event.inFFTBuffer.json", queue, inFFTBuffer, params.nl * params.samples_num );
#endif


    auto fft_prep_kernel_functor = cl::KernelFunctor< cl::Buffer, cl::Buffer, cl_uint, cl_uint, cl_uint, cl_int, cl_uint, cl_uint, cl_uint >{ 
        *( handler->program ), "FFTPrep_F" 
    };
    error = CL_SUCCESS;
    cl::EnqueueArgs fft_prep_eargs{ queue, cl::NullRange, cl::NDRange(N, params.kgd, params.nl), cl::NDRange(groupSize, 1, 1) };
    eventArray[2] = fft_prep_kernel_functor( 
        fft_prep_eargs, inFFTBuffer, midFFTBuffer, 
        params.log2N, 
        params.samples_num, 
        params.true_nihs, 
        params.nfgd_fu, 
        params.shgd, 
        params.ndec, 
        params.dlstr, 
        error 
    );

// Print midFFTBuffer
#ifdef ENABLE_DEBUG_COMPUTATIONS     
    writeBufferToJsonFile< cl_float2, std::complex< float > >( "event.FFTPrep_F.midFFTBuffer.json", queue, midFFTBuffer, params.nl * params.kgd * N );
#endif

    auto fft_fft_kernel_functor = cl::KernelFunctor< cl::Buffer, cl::Buffer, cl_uint, cl_uint >{ *( handler->program ), "FFT_FFT_F" };
    error = CL_SUCCESS;
    cl::EnqueueArgs fft_fft_eargs{ queue, cl::NullRange, cl::NDRange(groupSize, params.kgd, params.nl), cl::NDRange(groupSize, 1, 1) };
    eventArray[3] = fft_fft_kernel_functor( 
        fft_fft_eargs, midFFTBuffer, sinsBuffer, params.log2N, grouplog2,
        error 
    );

// Print midFFTBuffer
#ifdef ENABLE_DEBUG_COMPUTATIONS     
    writeBufferToJsonFile< cl_float2, std::complex< float > >( "event.FFT_FFT_F.midFFTBuffer.json", queue, midFFTBuffer, params.nl * params.kgd * N );
#endif

        
    auto fft_post_kernel_functor = cl::KernelFunctor< cl::Buffer, cl::Buffer, cl_uint, cl_int >{ *( handler->program ), "FFTPost_F" };
    error = CL_SUCCESS;
    cl::EnqueueArgs fft_post_eargs{ queue, cl::NullRange, cl::NDRange(params.kgrs, params.kgd, params.nl), cl::NullRange };
    eventArray[4] = fft_post_kernel_functor( 
        fft_post_eargs, midFFTBuffer, outFFTBuffer, params.log2N, params.n1grs,
        error 
    );

// Print outFFTBuffer
#ifdef ENABLE_DEBUG_COMPUTATIONS     
    writeBufferToJsonFile< cl_float2, std::complex< float > >( "event.FFTPost_F.outFFTBuffer.json", queue, outFFTBuffer, params.nl * params.kgrs * params.kgd );
#endif

        

    queue.enqueueReadBuffer(
        outFFTBuffer, CL_TRUE, 0,
        params.nl * params.kgd * params.kgrs * sizeof(cl_float2), outArray,
        NULL, &eventArray[5]);

    queue.finish();

    TimeResult time = {
        .writeStart             = eventArray[0].getProfilingInfo<CL_PROFILING_COMMAND_START>(),
        .writeEnd               = eventArray[0].getProfilingInfo<CL_PROFILING_COMMAND_END>(),
        .sineComputationStart   = eventArray[1].getProfilingInfo<CL_PROFILING_COMMAND_START>(),
        .sineComputationEnd     = eventArray[1].getProfilingInfo<CL_PROFILING_COMMAND_END>(),
        .fmSignFFTStart         = 0,
        .fmSignFFTEnd           = 0,
        .fmDataFFTStart         = 0,
        .fmDataFFTEnd           = 0,
        .FFTStart               = eventArray[2].getProfilingInfo<CL_PROFILING_COMMAND_START>(),
        .FFTEnd                 = eventArray[4].getProfilingInfo<CL_PROFILING_COMMAND_END>(),
        .readStart              = eventArray[5].getProfilingInfo<CL_PROFILING_COMMAND_START>(),
        .readEnd                = eventArray[5].getProfilingInfo<CL_PROFILING_COMMAND_END>(),
    };
    return time;
}
