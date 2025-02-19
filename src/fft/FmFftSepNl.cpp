#include <GpuFourier.h>

#include <config.h>

#ifdef ENABLE_DEBUG_COMPUTATIONS
#   include <debug_computations.h>
#endif

FmFftSepNl::FmFftSepNl( ProgramHandler * handler, const FftParams & params, cl_int2 * dataArray )  
:   FftInterface( handler, params, dataArray )
{}

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
