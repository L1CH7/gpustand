#include <GpuFourier.h>

#include <config.h>

#ifdef ENABLE_DEBUG_COMPUTATIONS
#   include <debug_computations.h>
#endif

FmFft::FmFft( ProgramHandler * handler, const FftParams & params, cl_int2 * dataArray )  
:   FftInterface( handler, params, dataArray )
{}

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
