#include <GpuFourier.h>

#include <config.h>

#ifdef ENABLE_DEBUG_COMPUTATIONS
#   include <debug_computations.h>
#endif

FmFft::FmFft( ProgramHandler * handler, const FftParams & params, cl_int2 * dataArray )  
:   FftInterface( handler, params, dataArray )
{
    std::cout<<"FmFft!\n";
}

TimeResult
FmFft::compute()
{
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
        params.nl * params.kgrs * N * sizeof( cl_float2 )
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

    cl::Event eventArray[11];
    cl_int error = CL_SUCCESS;

    queue.enqueueWriteBuffer(
        inSignBuffer, CL_TRUE, 0, 
        N * sizeof( cl_int ), ( const cl_int * )params.mseq.data(),
        NULL, &eventArray[0]
    );

    queue.enqueueWriteBuffer(
        inFFTBuffer, CL_TRUE, 0, 
        params.nl * params.samples_num * sizeof( cl_int2 ),
        dataArray, NULL, &eventArray[1]
    );


    auto sins_kernel_functor = cl::KernelFunctor< cl::Buffer >{ *( handler->program ), "getSinArrayTwoPi_F" };
    error = CL_SUCCESS;
    cl::EnqueueArgs sins_eargs{ queue, cl::NullRange, cl::NDRange( sinArrLen ), cl::NullRange };
    eventArray[2] = sins_kernel_functor( sins_eargs, sinsBuffer, error );

// Print sinsBuffer and inFFTBuffer
#ifdef ENABLE_DEBUG_COMPUTATIONS     
    writeBufferToJsonFile< cl_float, float >( "event.sinsBuffer.json", queue, sinsBuffer, sinArrLen );
    writeBufferToJsonFile< cl_int, int >( "event.inSignBuffer.json", queue, inSignBuffer, N );
    writeBufferToJsonFile< cl_int2, std::complex< int > >( "event.inFFTBuffer.json", queue, inFFTBuffer, params.nl * params.samples_num );
#endif

    // cl::Kernel signPrep(*(handler->program), "signPrep_F");
    // signPrep.setArg(0, inSignBuffer);
    // signPrep.setArg(1, outSignBuffer);
    // signPrep.setArg(2, sizeof(unsigned int), (void*)&params.log2N);
    // signPrep.setArg(3, sizeof(unsigned int), (void*)&params.true_nihs);

    // queue.enqueueNDRangeKernel(
    //     signPrep, cl::NullRange, cl::NDRange(N), cl::NullRange,
    //     NULL, &eventArray[3]);

    auto sign_prep_kernel_functor = cl::KernelFunctor< cl::Buffer, cl::Buffer, cl_uint, cl_uint >{ 
        *( handler->program ), "signPrep_F" 
    };
    error = CL_SUCCESS;
    cl::EnqueueArgs sign_prep_eargs{ queue, cl::NullRange, cl::NDRange( N ), cl::NullRange };
    eventArray[3] = sign_prep_kernel_functor( 
        sign_prep_eargs, inSignBuffer, outSignBuffer, 
        params.log2N, 
        params.true_nihs,
        error 
    );
// Print outSignBuffer
#ifdef ENABLE_DEBUG_COMPUTATIONS     
    writeBufferToJsonFile< cl_float2, std::complex< float > >( "event.signPrep_F.outSignBuffer.json", queue, outSignBuffer, N );
#endif
    // cl::Kernel signFFT(*(handler->program), "signFFT_F");
    // signFFT.setArg(0, outSignBuffer);
    // signFFT.setArg(1, sinsBuffer);
    // signFFT.setArg(2, sizeof(unsigned int), (void*)&params.log2N);
    // signFFT.setArg(3, sizeof(unsigned int), (void*)&grouplog2);
    // queue.enqueueNDRangeKernel(
    //     signFFT, cl::NullRange, cl::NDRange(groupSize), cl::NDRange(groupSize),
    //     NULL, &eventArray[4]);

    auto sign_fft_kernel_functor = cl::KernelFunctor< cl::Buffer, cl::Buffer, cl_uint, cl_uint >{ 
        *( handler->program ), "signFFT_F" 
    };
    error = CL_SUCCESS;
    cl::EnqueueArgs sign_fft_eargs{ queue, cl::NullRange, cl::NDRange( groupSize ), cl::NDRange( groupSize ) };
    eventArray[4] = sign_fft_kernel_functor( 
        sign_fft_eargs, outSignBuffer, sinsBuffer, 
        params.log2N, 
        grouplog2,
        error 
    );
// Print outSignBuffer
#ifdef ENABLE_DEBUG_COMPUTATIONS     
    writeBufferToJsonFile< cl_float2, std::complex< float > >( "event.signFFT_F.outSignBuffer.json", queue, outSignBuffer, N );
#endif
    // uint dlstr_ndec = params.dlstr / params.ndec;
    // cl::Kernel dataPrep(*(handler->program), "dataPrepSample_F");
    // dataPrep.setArg(0, inFFTBuffer);
    // dataPrep.setArg(1, outFFTBuffer);
    // dataPrep.setArg(2, sizeof(unsigned int), (void*)&params.log2N);
    // dataPrep.setArg(3, sizeof(unsigned int), (void*)&dlstr_ndec);
    // dataPrep.setArg(4, sizeof(unsigned int), (void*)&params.samples_num);
    // queue.enqueueNDRangeKernel(
    //     dataPrep, cl::NullRange, cl::NDRange(N, params.nl), cl::NullRange,
    //     NULL, &eventArray[5]);

    uint dlstr_ndec = params.dlstr / params.ndec;
    auto data_prep_kernel_functor = cl::KernelFunctor< cl::Buffer, cl::Buffer, cl_uint, cl_uint, cl_uint >{ 
        *( handler->program ), "dataPrepSample_F" 
    };
    error = CL_SUCCESS;
    cl::EnqueueArgs data_prep_eargs{ queue, cl::NullRange, cl::NDRange( N, params.nl ), cl::NullRange };
    eventArray[5] = data_prep_kernel_functor( 
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
    // cl::Kernel dataFFT(*(handler->program), "dataFFT_F");
    // dataFFT.setArg(0, outFFTBuffer);
    // dataFFT.setArg(1, sinsBuffer);
    // dataFFT.setArg(2, sizeof(unsigned int), (void*)&params.log2N);
    // dataFFT.setArg(3, sizeof(unsigned int), (void*)&grouplog2);
    // queue.enqueueNDRangeKernel(
    //     dataFFT, cl::NullRange, cl::NDRange(groupSize, params.nl),
    //     cl::NDRange(groupSize, 1), NULL, &eventArray[6]);

    auto data_fft_kernel_functor = cl::KernelFunctor< cl::Buffer, cl::Buffer, cl_uint, cl_uint >{ 
        *( handler->program ), "dataFFT_F" 
    };
    error = CL_SUCCESS;
    cl::EnqueueArgs data_fft_eargs{ queue, cl::NullRange, cl::NDRange( groupSize, params.nl ), cl::NDRange( groupSize, 1 ) };
    eventArray[6] = data_fft_kernel_functor( 
        data_fft_eargs, outFFTBuffer, sinsBuffer, 
        params.log2N, 
        grouplog2,
        error 
    );
// Print outFFTBuffer
#ifdef ENABLE_DEBUG_COMPUTATIONS     
    writeBufferToJsonFile< cl_float2, std::complex< float > >( "event.dataFFT_F.outFFTBuffer.json", queue, outFFTBuffer, params.nl * N );
#endif
    // int N2nihs = N/2/params.true_nihs;
    // cl::Kernel IFFTPrep(*(handler->program), "IFFTPrep_F");
    // IFFTPrep.setArg(0, outFFTBuffer);
    // IFFTPrep.setArg(1, outSignBuffer);
    // IFFTPrep.setArg(2, midIFFTBuffer);
    // IFFTPrep.setArg(3, sizeof(unsigned int), (void*)&params.log2N);
    // IFFTPrep.setArg(4, sizeof(unsigned int), (void*)&params.n1grs);
    // IFFTPrep.setArg(5, sizeof(unsigned int), (void*)&N2nihs);
    // queue.enqueueNDRangeKernel(
    //     IFFTPrep, cl::NullRange, cl::NDRange(N, params.kgrs, params.nl),
    //     cl::NullRange, NULL, &eventArray[7]);

    int N2nihs = N / 2 / params.true_nihs;
    auto ifft_prep_kernel_functor = cl::KernelFunctor< cl::Buffer, cl::Buffer, cl::Buffer, cl_uint, cl_uint, cl_uint >{ 
        *( handler->program ), "IFFTPrep_F" 
    };
    error = CL_SUCCESS;
    cl::EnqueueArgs ifft_prep_eargs{ queue, cl::NullRange, cl::NDRange( N, params.kgrs, params.nl ), cl::NullRange };
    eventArray[7] = ifft_prep_kernel_functor( 
        ifft_prep_eargs, outFFTBuffer, outSignBuffer, midIFFTBuffer,
        params.log2N, 
        params.n1grs, 
        N2nihs,
        error 
    );
// Print midIFFTBuffer
#ifdef ENABLE_DEBUG_COMPUTATIONS     
    writeBufferToJsonFile< cl_float2, std::complex< float > >( "event.IFFTPrep_F.midIFFTBuffer.json", queue, midIFFTBuffer, params.nl * params.kgrs * N );
#endif
    // cl::Kernel IFFT_FFT(*(handler->program), "IFFT_FFT_F");
    // IFFT_FFT.setArg(0, midIFFTBuffer);
    // IFFT_FFT.setArg(1, sinsBuffer);
    // IFFT_FFT.setArg(2, sizeof(unsigned int), (void*)&params.log2N);
    // IFFT_FFT.setArg(3, sizeof(unsigned int), (void*)&grouplog2);
    // queue.enqueueNDRangeKernel(
    //     IFFT_FFT, cl::NullRange,
    //     cl::NDRange(groupSize, params.kgrs, params.nl),
    //     cl::NDRange(groupSize, 1, 1), NULL, &eventArray[8]);

    auto ifft_fft_kernel_functor = cl::KernelFunctor< cl::Buffer, cl::Buffer, cl_uint, cl_uint >{ 
        *( handler->program ), "IFFT_FFT_F" 
    };
    error = CL_SUCCESS;
    cl::EnqueueArgs ifft_fft_eargs{ queue, cl::NullRange, cl::NDRange( groupSize, params.kgrs, params.nl ), cl::NDRange( groupSize, 1, 1 ) };
    eventArray[8] = ifft_fft_kernel_functor( 
        ifft_fft_eargs, midIFFTBuffer, sinsBuffer,
        params.log2N, 
        grouplog2,
        error 
    );
// Print midIFFTBuffer
#ifdef ENABLE_DEBUG_COMPUTATIONS     
    writeBufferToJsonFile< cl_float2, std::complex< float > >( "event.IFFT_FFT_F.midIFFTBuffer.json", queue, midIFFTBuffer, params.nl * params.kgrs * N );
#endif
    // cl::Kernel IFFTPost(*(handler->program), "IFFTPost_F");
    // IFFTPost.setArg(0, midIFFTBuffer);
    // IFFTPost.setArg(1, outIFFTBuffer);
    // IFFTPost.setArg(2, sizeof(unsigned int), (void*)&params.log2N);
    // IFFTPost.setArg(3, sizeof(unsigned int), (void*)&params.nfgd_fu);
    // IFFTPost.setArg(4, sizeof(unsigned int), (void*)&params.shgd);
    // IFFTPost.setArg(5, sizeof(unsigned int), (void*)&params.ndec);
    // queue.enqueueNDRangeKernel(
    //     IFFTPost, cl::NullRange,
    //     cl::NDRange(params.kgd, params.kgrs, params.nl),
    //     cl::NullRange, NULL, &eventArray[9]);

    auto ifft_post_kernel_functor = cl::KernelFunctor< cl::Buffer, cl::Buffer, cl_uint, cl_uint, cl_uint, cl_uint >{ 
        *( handler->program ), "IFFTPost_F" 
    };
    error = CL_SUCCESS;
    cl::EnqueueArgs ifft_post_eargs{ queue, cl::NullRange, cl::NDRange( params.kgd, params.kgrs, params.nl ), cl::NullRange };
    eventArray[9] = ifft_post_kernel_functor( 
        ifft_post_eargs, midIFFTBuffer, outIFFTBuffer,
        params.log2N, 
        params.nfgd_fu, 
        params.shgd, 
        params.ndec, 
        error 
    );
// Print outIFFTBuffer
#ifdef ENABLE_DEBUG_COMPUTATIONS     
    writeBufferToJsonFile< cl_float2, std::complex< float > >( "event.IFFTPost_F.outIFFTBuffer.json", queue, outIFFTBuffer, params.nl * params.kgrs * params.kgd );
#endif

    queue.enqueueReadBuffer(
        outIFFTBuffer, CL_TRUE, 0,
        params.nl * params.kgrs * params.kgd * sizeof(cl_float2),
        outArray,
        NULL, &eventArray[10]
    );

    queue.finish();

    TimeResult time = {
        .writeStart             = eventArray[0].getProfilingInfo<CL_PROFILING_COMMAND_START>(),
        .writeEnd               = eventArray[1].getProfilingInfo<CL_PROFILING_COMMAND_END>(),
        .sineComputationStart   = eventArray[3].getProfilingInfo<CL_PROFILING_COMMAND_START>(),
        .sineComputationEnd     = eventArray[3].getProfilingInfo<CL_PROFILING_COMMAND_END>(),
        .fmSignFFTStart         = eventArray[4].getProfilingInfo<CL_PROFILING_COMMAND_START>(),
        .fmSignFFTEnd           = eventArray[4].getProfilingInfo<CL_PROFILING_COMMAND_END>(),
        .fmDataFFTStart         = eventArray[5].getProfilingInfo<CL_PROFILING_COMMAND_START>(),
        .fmDataFFTEnd           = eventArray[6].getProfilingInfo<CL_PROFILING_COMMAND_END>(),
        .FFTStart               = eventArray[7].getProfilingInfo<CL_PROFILING_COMMAND_START>(),
        .FFTEnd                 = eventArray[9].getProfilingInfo<CL_PROFILING_COMMAND_END>(),
        .readStart              = eventArray[10].getProfilingInfo<CL_PROFILING_COMMAND_START>(),
        .readEnd                = eventArray[10].getProfilingInfo<CL_PROFILING_COMMAND_END>(),
    };
    return time;
}
