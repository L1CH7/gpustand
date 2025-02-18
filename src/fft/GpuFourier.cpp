#include <GpuFourier.h>

#include <config.h>

#ifdef ENABLE_DEBUG_COMPUTATIONS
#   include <debug_computations.h>
#endif

TimeResult AmFft::compute()
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
        *(handler->context), 
#ifdef ENABLE_DEBUG_COMPUTATIONS
        CL_MEM_READ_WRITE,
#else
        CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS,
#endif
        sinArrLen * sizeof(cl_float));
        

    cl::Buffer inFFTBuffer(
        *(handler->context), 
#ifdef ENABLE_DEBUG_COMPUTATIONS
        CL_MEM_READ_ONLY,
#else
        CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY,
#endif
        params.nl * params.samples_num * sizeof(cl_int2));


    cl::Buffer midFFTBuffer(
        *(handler->context), 
#ifdef ENABLE_DEBUG_COMPUTATIONS
        CL_MEM_READ_WRITE,
#else
        CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS,
#endif
          params.nl * params.kgd * N * sizeof(cl_float2));


    cl::Buffer outFFTBuffer(
        *(handler->context), CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY,
        params.nl * params.kgrs * params.kgd * sizeof(cl_float2));


    cl::CommandQueue queue(
        *(handler->context), *(handler->device), queue_properties, NULL);

    cl::Event eventArray[6];
    cl_int error = CL_SUCCESS;

    queue.enqueueWriteBuffer(
        inFFTBuffer, CL_TRUE, 0,
        params.nl * params.samples_num * sizeof(cl_int2),
        dataArray, NULL, &eventArray[0]);

    auto sins_kernel_functor = cl::KernelFunctor< cl::Buffer >{ *( handler->program ), "getSinArrayTwoPi_F" };
    error = CL_SUCCESS;
    cl::EnqueueArgs sins_eargs{ queue, cl::NullRange, cl::NDRange(sinArrLen), cl::NullRange };
    eventArray[1] = sins_kernel_functor( sins_eargs, sinsBuffer, error );

// Print sinsBuffer and inFFTBuffer
#ifdef ENABLE_DEBUG_COMPUTATIONS     
    writeBufferToJsonFile< cl_float, float >( "event_getSinArrayTwoPi_F.json", queue, sinsBuffer, sinArrLen );
    writeBufferToJsonFile< cl_int2, std::complex< int > >( "event_in_buf.json", queue, inFFTBuffer, params.nl * params.samples_num );
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
    writeBufferToJsonFile< cl_float2, std::complex< float > >( "event_FFTPrep_F_mid.json", queue, midFFTBuffer, params.nl * params.kgd * N );
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
    writeBufferToJsonFile< cl_float2, std::complex< float > >( "event_FFT_FFT_F_mid.json", queue, midFFTBuffer, params.nl * params.kgd * N );
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
    writeBufferToJsonFile< cl_float2, std::complex< float > >( "event_FFTPost_F_out.json", queue, outFFTBuffer, params.nl * params.kgrs * params.kgd );
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

TimeResult FmFft::compute()
{
    const int sinArrLen = 524288; //2^19
    uint32_t N = 1 << params.log2N;

    uint32_t grouplog2 = 8;
    if ((params.log2N - 1) < grouplog2)
        grouplog2 = params.log2N - 1;
    uint32_t groupSize = 1 << grouplog2;

    cl_command_queue_properties queue_properties = CL_QUEUE_PROFILING_ENABLE;

    cl::Buffer sinsBuffer(
        *(handler->context), CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS,
        sinArrLen * sizeof(float));

    cl::Buffer inSignBuffer(
        *(handler->context), CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY,
        N * sizeof(int));
    cl::Buffer outSignBuffer(
        *(handler->context), CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS,
        N * sizeof(cl_float2));

    cl::Buffer inFFTBuffer(
        *(handler->context), CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY,
        params.nl * params.samples_num * sizeof(cl_int2));
    cl::Buffer outFFTBuffer(
        *(handler->context), CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS,
        params.nl * N * sizeof(cl_float2));

    cl::Buffer midIFFTBuffer(
        *(handler->context), CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS,
        params.nl * params.kgrs * N * sizeof(cl_float2));
    cl::Buffer outIFFTBuffer(
        *(handler->context), CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY,
        params.nl * params.kgrs * params.kgd * sizeof(cl_float2));

    cl::CommandQueue queue(
        *(handler->context), *(handler->device), queue_properties, NULL);
    cl::Event eventArray[11];

    queue.enqueueWriteBuffer(
        inSignBuffer, CL_TRUE, 0, N * sizeof(int), ( const cl_int * )&params.mseq.front(),
        NULL, &eventArray[0]);
    queue.enqueueWriteBuffer(
        inFFTBuffer, CL_TRUE, 0, 
        params.nl * params.samples_num * sizeof(cl_int2),
        dataArray, NULL, &eventArray[1]);

    cl::Kernel sinKernel(*(handler->program), "getSinArrayTwoPi_F");
    sinKernel.setArg(0, sinsBuffer);
    queue.enqueueNDRangeKernel(
        sinKernel, cl::NullRange, cl::NDRange(sinArrLen), cl::NullRange,
        NULL, &eventArray[2]);

    cl::Kernel signPrep(*(handler->program), "signPrep_F");
    signPrep.setArg(0, inSignBuffer);
    signPrep.setArg(1, outSignBuffer);
    signPrep.setArg(2, sizeof(unsigned int), (void*)&params.log2N);
    signPrep.setArg(3, sizeof(unsigned int), (void*)&params.true_nihs);
    queue.enqueueNDRangeKernel(
        signPrep, cl::NullRange, cl::NDRange(N), cl::NullRange,
        NULL, &eventArray[3]);

    cl::Kernel signFFT(*(handler->program), "signFFT_F");
    signFFT.setArg(0, outSignBuffer);
    signFFT.setArg(1, sinsBuffer);
    signFFT.setArg(2, sizeof(unsigned int), (void*)&params.log2N);
    signFFT.setArg(3, sizeof(unsigned int), (void*)&grouplog2);
    queue.enqueueNDRangeKernel(
        signFFT, cl::NullRange, cl::NDRange(groupSize), cl::NDRange(groupSize),
        NULL, &eventArray[4]);

    uint dlstr_ndec = params.dlstr / params.ndec;
    cl::Kernel dataPrep(*(handler->program), "dataPrepSample_F");
    dataPrep.setArg(0, inFFTBuffer);
    dataPrep.setArg(1, outFFTBuffer);
    dataPrep.setArg(2, sizeof(unsigned int), (void*)&params.log2N);
    dataPrep.setArg(3, sizeof(unsigned int), (void*)&dlstr_ndec);
    dataPrep.setArg(4, sizeof(unsigned int), (void*)&params.samples_num);
    queue.enqueueNDRangeKernel(
        dataPrep, cl::NullRange, cl::NDRange(N, params.nl), cl::NullRange,
        NULL, &eventArray[5]);
    cl::Kernel dataFFT(*(handler->program), "dataFFT_F");
    dataFFT.setArg(0, outFFTBuffer);
    dataFFT.setArg(1, sinsBuffer);
    dataFFT.setArg(2, sizeof(unsigned int), (void*)&params.log2N);
    dataFFT.setArg(3, sizeof(unsigned int), (void*)&grouplog2);
    queue.enqueueNDRangeKernel(
        dataFFT, cl::NullRange, cl::NDRange(groupSize, params.nl),
        cl::NDRange(groupSize, 1), NULL, &eventArray[6]);

    int N2nihs = N/2/params.true_nihs;
    cl::Kernel IFFTPrep(*(handler->program), "IFFTPrep_F");
    IFFTPrep.setArg(0, outFFTBuffer);
    IFFTPrep.setArg(1, outSignBuffer);
    IFFTPrep.setArg(2, midIFFTBuffer);
    IFFTPrep.setArg(3, sizeof(unsigned int), (void*)&params.log2N);
    IFFTPrep.setArg(4, sizeof(unsigned int), (void*)&params.n1grs);
    IFFTPrep.setArg(5, sizeof(unsigned int), (void*)&N2nihs);
    queue.enqueueNDRangeKernel(
        IFFTPrep, cl::NullRange, cl::NDRange(N, params.kgrs, params.nl),
        cl::NullRange, NULL, &eventArray[7]);
    cl::Kernel IFFT_FFT(*(handler->program), "IFFT_FFT_F");
    IFFT_FFT.setArg(0, midIFFTBuffer);
    IFFT_FFT.setArg(1, sinsBuffer);
    IFFT_FFT.setArg(2, sizeof(unsigned int), (void*)&params.log2N);
    IFFT_FFT.setArg(3, sizeof(unsigned int), (void*)&grouplog2);
    queue.enqueueNDRangeKernel(
        IFFT_FFT, cl::NullRange,
        cl::NDRange(groupSize, params.kgrs, params.nl),
        cl::NDRange(groupSize, 1, 1), NULL, &eventArray[8]);
    cl::Kernel IFFTPost(*(handler->program), "IFFTPost_F");
    IFFTPost.setArg(0, midIFFTBuffer);
    IFFTPost.setArg(1, outIFFTBuffer);
    IFFTPost.setArg(2, sizeof(unsigned int), (void*)&params.log2N);
    IFFTPost.setArg(3, sizeof(unsigned int), (void*)&params.nfgd_fu);
    IFFTPost.setArg(4, sizeof(unsigned int), (void*)&params.shgd);
    IFFTPost.setArg(5, sizeof(unsigned int), (void*)&params.ndec);
    queue.enqueueNDRangeKernel(
        IFFTPost, cl::NullRange,
        cl::NDRange(params.kgd, params.kgrs, params.nl),
        cl::NullRange, NULL, &eventArray[9]);

    queue.enqueueReadBuffer(
        outIFFTBuffer, CL_TRUE, 0,
        params.nl * params.kgrs * params.kgd * sizeof(cl_float2),
        outArray,
        NULL, &eventArray[10]);

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

TimeResult FmFftSepNl::compute()
{
    const int sinArrLen = 524288; //2^19
    uint32_t N = 1 << params.log2N;

    uint32_t grouplog2 = 8;
    if ((params.log2N - 1) < grouplog2)
        grouplog2 = params.log2N - 1;
    uint32_t groupSize = 1 << grouplog2;

    cl_command_queue_properties queue_properties = CL_QUEUE_PROFILING_ENABLE;

    cl::Buffer sinsBuffer(
        *(handler->context), CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS,
        sinArrLen * sizeof(float));

    cl::Buffer inSignBuffer(
        *(handler->context), CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY,
        N * sizeof(int));
    cl::Buffer outSignBuffer(
        *(handler->context), CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS,
        N * sizeof(cl_float2));

    cl::Buffer inFFTBuffer(
        *(handler->context), CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY,
        params.nl * params.samples_num * sizeof(cl_int2));
    cl::Buffer outFFTBuffer(
        *(handler->context), CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS,
        params.nl * N * sizeof(cl_float2));

    cl::Buffer midIFFTBuffer(
        *(handler->context), CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS,
        params.kgrs * N * sizeof(cl_float2));
    cl::Buffer outIFFTBuffer(
        *(handler->context), CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY,
        params.nl * params.kgrs * params.kgd * sizeof(cl_float2));

    cl::CommandQueue queue(
        *(handler->context), *(handler->device), queue_properties, NULL);
    cl::Event eventArray[8 + params.nl*3];
    int eventcounter = 0;

    queue.enqueueWriteBuffer(
        inSignBuffer, CL_TRUE, 0, N * sizeof(int), ( const cl_int * )&params.mseq.front(), NULL,
        &eventArray[/* 0 */eventcounter++]);
    queue.enqueueWriteBuffer(
        inFFTBuffer, CL_TRUE, 0, 
        params.nl * params.samples_num * sizeof(cl_int2),
        dataArray, NULL, &eventArray[/* 1 */eventcounter++]);

    cl::Kernel sinKernel(*(handler->program), "getSinArrayTwoPi_F");
    sinKernel.setArg(0, sinsBuffer);
    queue.enqueueNDRangeKernel(
        sinKernel, cl::NullRange, cl::NDRange(sinArrLen), cl::NullRange,
        NULL, &eventArray[/* 2 */eventcounter++]);

    cl::Kernel signPrep(*(handler->program), "signPrep_F");
    signPrep.setArg(0, inSignBuffer);
    signPrep.setArg(1, outSignBuffer);
    signPrep.setArg(2, sizeof(unsigned int), (void*)&params.log2N);
    signPrep.setArg(3, sizeof(unsigned int), (void*)&params.true_nihs);
    queue.enqueueNDRangeKernel(
        signPrep, cl::NullRange, cl::NDRange(N), cl::NullRange,
        NULL, &eventArray[/* 3 */eventcounter++]);
    cl::Kernel signFFT(*(handler->program), "signFFT_F");
    signFFT.setArg(0, outSignBuffer);
    signFFT.setArg(1, sinsBuffer);
    signFFT.setArg(2, sizeof(unsigned int), (void*)&params.log2N);
    signFFT.setArg(3, sizeof(unsigned int), (void*)&grouplog2);
    queue.enqueueNDRangeKernel(
        signFFT, cl::NullRange, cl::NDRange(groupSize), cl::NDRange(groupSize),
        NULL, &eventArray[/* 4 */eventcounter++]);

    uint dlstr_ndec = params.dlstr / params.ndec;
    cl::Kernel dataPrep(*(handler->program), "dataPrepSample_F");
    dataPrep.setArg(0, inFFTBuffer);
    dataPrep.setArg(1, outFFTBuffer);
    dataPrep.setArg(2, sizeof(unsigned int), (void*)&params.log2N);
    dataPrep.setArg(3, sizeof(unsigned int), (void*)&dlstr_ndec);
    dataPrep.setArg(4, sizeof(unsigned int), (void*)&params.samples_num);
    queue.enqueueNDRangeKernel(
        dataPrep, cl::NullRange, cl::NDRange(N, params.nl), cl::NullRange, NULL,
        &eventArray[/* 5 */eventcounter++]);
    cl::Kernel dataFFT(*(handler->program), "dataFFT_F");
    dataFFT.setArg(0, outFFTBuffer);
    dataFFT.setArg(1, sinsBuffer);
    dataFFT.setArg(2, sizeof(unsigned int), (void*)&params.log2N);
    dataFFT.setArg(3, sizeof(unsigned int), (void*)&grouplog2);
    queue.enqueueNDRangeKernel(
        dataFFT, cl::NullRange, cl::NDRange(groupSize, params.nl),
        cl::NDRange(groupSize, 1), NULL, &eventArray[eventcounter++]);

    for (int nl = 0; nl < params.nl; nl++) {
        int N2nihs = N/2/params.true_nihs;
        cl::Kernel IFFTPrepsepNl(*(handler->program), "IFFTPrepsepNl_F");
        IFFTPrepsepNl.setArg(0, outFFTBuffer);
        IFFTPrepsepNl.setArg(1, outSignBuffer);
        IFFTPrepsepNl.setArg(2, midIFFTBuffer);
        IFFTPrepsepNl.setArg(3, sizeof(unsigned int), (void*)&params.log2N);
        IFFTPrepsepNl.setArg(4, sizeof(unsigned int), (void*)&params.n1grs);
        IFFTPrepsepNl.setArg(5, sizeof(unsigned int), (void*)&N2nihs);
        IFFTPrepsepNl.setArg(6, sizeof(unsigned int), (void*)&nl);
        queue.enqueueNDRangeKernel(
            IFFTPrepsepNl, cl::NullRange, cl::NDRange(N, params.kgrs),
            cl::NullRange, NULL, &eventArray[eventcounter++]);
        cl::Kernel IFFT_FFTsepNl(*(handler->program), "IFFT_FFTsepNl_F");
        IFFT_FFTsepNl.setArg(0, midIFFTBuffer);
        IFFT_FFTsepNl.setArg(1, sinsBuffer);
        IFFT_FFTsepNl.setArg(2, sizeof(unsigned int), (void*)&params.log2N);
        IFFT_FFTsepNl.setArg(3, sizeof(unsigned int), (void*)&grouplog2);
        queue.enqueueNDRangeKernel(
            IFFT_FFTsepNl, cl::NullRange, cl::NDRange(groupSize, params.kgrs),
            cl::NDRange(groupSize, 1), NULL, &eventArray[eventcounter++]);
        cl::Kernel IFFTPostsepNl(*(handler->program), "IFFTPostsepNl_F");
        IFFTPostsepNl.setArg(0, midIFFTBuffer);
        IFFTPostsepNl.setArg(1, outIFFTBuffer);
        IFFTPostsepNl.setArg(2, sizeof(unsigned int), (void*)&params.log2N);
        IFFTPostsepNl.setArg(3, sizeof(unsigned int), (void*)&params.nfgd_fu);
        IFFTPostsepNl.setArg(4, sizeof(unsigned int), (void*)&params.shgd);
        IFFTPostsepNl.setArg(5, sizeof(unsigned int), (void*)&params.ndec);
        IFFTPostsepNl.setArg(6, sizeof(unsigned int), (void*)&nl);
        queue.enqueueNDRangeKernel(
            IFFTPostsepNl, cl::NullRange, cl::NDRange(params.kgd, params.kgrs),
            cl::NullRange, NULL, &eventArray[eventcounter++]);
    }

    queue.enqueueReadBuffer(
        outIFFTBuffer, CL_TRUE, 0,
        params.nl * params.kgrs * params.kgd * sizeof(cl_float2), outArray,
        NULL, &eventArray[eventcounter++]);

    queue.finish();

    int shift = 6 + params.nl * 3;
    TimeResult time = {
        .writeStart             = eventArray[0].getProfilingInfo<CL_PROFILING_COMMAND_START>(),
        .writeEnd               = eventArray[1].getProfilingInfo<CL_PROFILING_COMMAND_END>(),
        .sineComputationStart   = eventArray[2].getProfilingInfo<CL_PROFILING_COMMAND_START>(),
        .sineComputationEnd     = eventArray[2].getProfilingInfo<CL_PROFILING_COMMAND_END>(),
        .fmSignFFTStart         = eventArray[3].getProfilingInfo<CL_PROFILING_COMMAND_START>(),
        .fmSignFFTEnd           = eventArray[4].getProfilingInfo<CL_PROFILING_COMMAND_END>(),
        .fmDataFFTStart         = eventArray[5].getProfilingInfo<CL_PROFILING_COMMAND_START>(),
        .fmDataFFTEnd           = eventArray[6].getProfilingInfo<CL_PROFILING_COMMAND_END>(),
        .FFTStart               = eventArray[7].getProfilingInfo<CL_PROFILING_COMMAND_START>(),
        .FFTEnd                 = eventArray[shift].getProfilingInfo<CL_PROFILING_COMMAND_END>(),
        .readStart              = eventArray[shift+1].getProfilingInfo<CL_PROFILING_COMMAND_START>(),
        .readEnd                = eventArray[shift+1].getProfilingInfo<CL_PROFILING_COMMAND_END>(),
    };
    return time;
}


// int main()
// {
//     FftParams p;
//     p.is_am = false;
//     p.is_am = true;
//     FftCreator fft( p );
//     fft.compute();
    
//     return 0;
// }