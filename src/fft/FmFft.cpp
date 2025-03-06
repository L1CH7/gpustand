#include <GpuFourier.h>

#include <config.h>

#ifdef ENABLE_DEBUG_COMPUTATIONS
#   include <debug_computations.h>
#endif

FmFft::FmFft( std::shared_ptr< ProgramHandler > handler, FftData & data )
:   FftInterface( handler, data )
{
    std::cout<<"FM FFT instance c-tor!\n";
}

TimeResult
FmFft::compute()
{
    std::stringstream ss;
    ss << "FmFft computing! Test:" << params_.test_name << std::endl;
    std::cout << ss.str();
    const uint32_t sinArrLen = 524288; //2^19
    uint32_t N = 1 << params_.log2N;

    uint32_t group_log2 = 8;
    if ((params_.log2N - 1) < group_log2)
        group_log2 = params_.log2N - 1;
    uint32_t group_size = 1 << group_log2;

// Print this->data_array
#ifdef ENABLE_DEBUG_COMPUTATIONS     
    writePtrArrayToJsonFile< cl_int2, std::complex< int > >( "event_params_dataArray.json", data_array, params_.nl * params_.samples_num );
#endif

    cl_command_queue_properties queue_properties = CL_QUEUE_PROFILING_ENABLE;

    cl::Buffer sinsBuffer(
        *( handler_->context ), 
#ifdef ENABLE_DEBUG_COMPUTATIONS
        CL_MEM_READ_WRITE,
#else
        CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS,
#endif
        sinArrLen * sizeof( cl_float )
    );
        
    cl::Buffer inSignBuffer(
        *( handler_->context ), 
#ifdef ENABLE_DEBUG_COMPUTATIONS
        CL_MEM_READ_ONLY,
#else
        CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY,
#endif
        N * sizeof( cl_int )
    );

    cl::Buffer outSignBuffer(
        *( handler_->context ), 
#ifdef ENABLE_DEBUG_COMPUTATIONS
        CL_MEM_READ_WRITE,
#else
        CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS,
#endif
        N * sizeof( cl_float2 )
    );

    cl::Buffer inFFTBuffer(
        *( handler_->context ), 
#ifdef ENABLE_DEBUG_COMPUTATIONS
        CL_MEM_READ_ONLY,
#else
        CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY,
#endif
        params_.nl * params_.samples_num * sizeof( cl_int2 )
    );

    cl::Buffer outFFTBuffer(
        *( handler_->context ), 
#ifdef ENABLE_DEBUG_COMPUTATIONS
        CL_MEM_READ_WRITE,
#else
        CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS,
#endif
        params_.nl * N * sizeof( cl_float2 )
    );

    cl::Buffer midIFFTBuffer(
        *( handler_->context ), 
#ifdef ENABLE_DEBUG_COMPUTATIONS
        CL_MEM_READ_WRITE,
#else
        CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS,
#endif
        params_.nl * params_.kgrs * N * sizeof( cl_float2 )
    );

    cl::Buffer outIFFTBuffer(
        *( handler_->context ), 
#ifdef ENABLE_DEBUG_COMPUTATIONS
        CL_MEM_READ_WRITE,
#else
        CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY,
#endif
        params_.nl * params_.kgrs * params_.kgd * sizeof( cl_float2 )
    );

    cl::CommandQueue queue(
        *( handler_->context ), *( handler_->device ), queue_properties, NULL 
    );

    cl::Event eventArray[11];
    cl_int error = CL_SUCCESS;

    queue.enqueueWriteBuffer(
        inSignBuffer, CL_TRUE, 0, 
        N * sizeof( cl_int ), 
        reinterpret_cast< cl_int * >( mseq_.data() ),
        NULL, &eventArray[0]
    );
    // mseq_.clear(); // RAM cleanup
    // mseq_.shrink_to_fit();
    // std::vector< int >().swap( mseq_ );

    queue.enqueueWriteBuffer(
        inFFTBuffer, CL_TRUE, 0, 
        params_.nl * params_.samples_num * sizeof( cl_int2 ),
        reinterpret_cast< cl_int2 * >( data_array_.data() ), 
        NULL, &eventArray[1]
    );
    // data_array_.clear(); // RAM cleanup
    // data_array_.shrink_to_fit();

    auto sins_kernel_functor = cl::KernelFunctor< cl::Buffer >{ *( handler_->program ), "getSinArrayTwoPi_F" };
    error = CL_SUCCESS;
    cl::EnqueueArgs sins_eargs{ queue, cl::NullRange, cl::NDRange( sinArrLen ), cl::NullRange };
    eventArray[2] = sins_kernel_functor( sins_eargs, sinsBuffer, error );

// Print sinsBuffer and inFFTBuffer
#ifdef ENABLE_DEBUG_COMPUTATIONS     
    writeBufferToJsonFile< cl_float, float >( "event.sinsBuffer.json", queue, sinsBuffer, sinArrLen );
    writeBufferToJsonFile< cl_int, int >( "event.inSignBuffer.json", queue, inSignBuffer, N );
    writeBufferToJsonFile< cl_int2, std::complex< int > >( "event.inFFTBuffer.json", queue, inFFTBuffer, params_.nl * params_.samples_num );
#endif

    auto sign_prep_kernel_functor = cl::KernelFunctor< cl::Buffer, cl::Buffer, cl_uint, cl_uint >{ 
        *( handler_->program ), "signPrep_F" 
    };
    error = CL_SUCCESS;
    cl::EnqueueArgs sign_prep_eargs{ queue, cl::NullRange, cl::NDRange( N ), cl::NullRange };
    eventArray[3] = sign_prep_kernel_functor( 
        sign_prep_eargs, inSignBuffer, outSignBuffer, 
        params_.log2N, 
        params_.true_nihs,
        error 
    );
// Print outSignBuffer
#ifdef ENABLE_DEBUG_COMPUTATIONS     
    writeBufferToJsonFile< cl_float2, std::complex< float > >( "event.signPrep_F.outSignBuffer.json", queue, outSignBuffer, N );
#endif

    auto sign_fft_kernel_functor = cl::KernelFunctor< cl::Buffer, cl::Buffer, cl_uint, cl_uint >{ 
        *( handler_->program ), "signFFT_F" 
    };
    error = CL_SUCCESS;
    cl::EnqueueArgs sign_fft_eargs{ queue, cl::NullRange, cl::NDRange( group_size ), cl::NDRange( group_size ) };
    eventArray[4] = sign_fft_kernel_functor( 
        sign_fft_eargs, outSignBuffer, sinsBuffer, 
        params_.log2N, 
        group_log2,
        error 
    );
// Print outSignBuffer
#ifdef ENABLE_DEBUG_COMPUTATIONS     
    writeBufferToJsonFile< cl_float2, std::complex< float > >( "event.signFFT_F.outSignBuffer.json", queue, outSignBuffer, N );
#endif

    uint dlstr_ndec = params_.dlstr / params_.ndec;
    auto data_prep_kernel_functor = cl::KernelFunctor< cl::Buffer, cl::Buffer, cl_uint, cl_uint, cl_uint >{ 
        *( handler_->program ), "dataPrepSample_F" 
    };
    error = CL_SUCCESS;
    cl::EnqueueArgs data_prep_eargs{ queue, cl::NullRange, cl::NDRange( N, params_.nl ), cl::NullRange };
    eventArray[5] = data_prep_kernel_functor( 
        data_prep_eargs, inFFTBuffer, outFFTBuffer, 
        params_.log2N, 
        dlstr_ndec,
        params_.samples_num, 
        error 
    );
// Print outFFTBuffer
#ifdef ENABLE_DEBUG_COMPUTATIONS     
    writeBufferToJsonFile< cl_float2, std::complex< float > >( "event.dataPrepSample_F.outFFTBuffer.json", queue, outFFTBuffer, params_.nl * N );
#endif

    auto data_fft_kernel_functor = cl::KernelFunctor< cl::Buffer, cl::Buffer, cl_uint, cl_uint >{ 
        *( handler_->program ), "dataFFT_F" 
    };
    error = CL_SUCCESS;
    cl::EnqueueArgs data_fft_eargs{ queue, cl::NullRange, cl::NDRange( group_size, params_.nl ), cl::NDRange( group_size, 1 ) };
    eventArray[6] = data_fft_kernel_functor( 
        data_fft_eargs, outFFTBuffer, sinsBuffer, 
        params_.log2N, 
        group_log2,
        error 
    );
// Print outFFTBuffer
#ifdef ENABLE_DEBUG_COMPUTATIONS     
    writeBufferToJsonFile< cl_float2, std::complex< float > >( "event.dataFFT_F.outFFTBuffer.json", queue, outFFTBuffer, params_.nl * N );
#endif

    int N2nihs = N / 2 / params_.true_nihs;
    auto ifft_prep_kernel_functor = cl::KernelFunctor< cl::Buffer, cl::Buffer, cl::Buffer, cl_uint, cl_uint, cl_uint >{ 
        *( handler_->program ), "IFFTPrep_F" 
    };
    error = CL_SUCCESS;
    cl::EnqueueArgs ifft_prep_eargs{ queue, cl::NullRange, cl::NDRange( N, params_.kgrs, params_.nl ), cl::NullRange };
    eventArray[7] = ifft_prep_kernel_functor( 
        ifft_prep_eargs, outFFTBuffer, outSignBuffer, midIFFTBuffer,
        params_.log2N, 
        params_.n1grs, 
        N2nihs,
        error 
    );
// Print midIFFTBuffer
#ifdef ENABLE_DEBUG_COMPUTATIONS     
    writeBufferToJsonFile< cl_float2, std::complex< float > >( "event.IFFTPrep_F.midIFFTBuffer.json", queue, midIFFTBuffer, params_.nl * params_.kgrs * N );
#endif

    auto ifft_fft_kernel_functor = cl::KernelFunctor< cl::Buffer, cl::Buffer, cl_uint, cl_uint >{ 
        *( handler_->program ), "IFFT_FFT_F" 
    };
    error = CL_SUCCESS;
    cl::EnqueueArgs ifft_fft_eargs{ queue, cl::NullRange, cl::NDRange( group_size, params_.kgrs, params_.nl ), cl::NDRange( group_size, 1, 1 ) };
    eventArray[8] = ifft_fft_kernel_functor( 
        ifft_fft_eargs, midIFFTBuffer, sinsBuffer,
        params_.log2N, 
        group_log2,
        error 
    );
// Print midIFFTBuffer
#ifdef ENABLE_DEBUG_COMPUTATIONS     
    writeBufferToJsonFile< cl_float2, std::complex< float > >( "event.IFFT_FFT_F.midIFFTBuffer.json", queue, midIFFTBuffer, params_.nl * params_.kgrs * N );
#endif

    auto ifft_post_kernel_functor = cl::KernelFunctor< cl::Buffer, cl::Buffer, cl_uint, cl_uint, cl_uint, cl_uint >{ 
        *( handler_->program ), "IFFTPost_F" 
    };
    error = CL_SUCCESS;
    cl::EnqueueArgs ifft_post_eargs{ queue, cl::NullRange, cl::NDRange( params_.kgd, params_.kgrs, params_.nl ), cl::NullRange };
    eventArray[9] = ifft_post_kernel_functor( 
        ifft_post_eargs, midIFFTBuffer, outIFFTBuffer,
        params_.log2N, 
        params_.nfgd_fu, 
        params_.shgd, 
        params_.ndec, 
        error 
    );
// Print outIFFTBuffer
#ifdef ENABLE_DEBUG_COMPUTATIONS     
    writeBufferToJsonFile< cl_float2, std::complex< float > >( "event.IFFTPost_F.outIFFTBuffer.json", queue, outIFFTBuffer, params_.nl * params_.kgrs * params_.kgd );
#endif

    queue.enqueueReadBuffer(
        outIFFTBuffer, CL_TRUE, 0,
        params_.nl * params_.kgrs * params_.kgd * sizeof( cl_float2 ),
        reinterpret_cast< cl_float2 * >( out_array_.data() ),
        NULL, &eventArray[10]
    );

    queue.finish();

    // RAM cleanup
    std::vector< std::complex< int > >().swap( data_array_ );
    std::vector< int >().swap( mseq_ );

    const std::scoped_lock< std::mutex > l( m_ );
    TimeResult time = {
        .write_start            = eventArray[0].getProfilingInfo<CL_PROFILING_COMMAND_START>(),
        .write_end              = eventArray[1].getProfilingInfo<CL_PROFILING_COMMAND_END>(),
        .sine_computation_start = eventArray[2].getProfilingInfo<CL_PROFILING_COMMAND_START>(),
        .sine_computation_end   = eventArray[2].getProfilingInfo<CL_PROFILING_COMMAND_END>(),
        .fm_sign_fft_start      = eventArray[3].getProfilingInfo<CL_PROFILING_COMMAND_START>(),
        .fm_sign_fft_end        = eventArray[4].getProfilingInfo<CL_PROFILING_COMMAND_END>(),
        .fm_data_fft_start      = eventArray[5].getProfilingInfo<CL_PROFILING_COMMAND_START>(),
        .fm_data_fft_end        = eventArray[6].getProfilingInfo<CL_PROFILING_COMMAND_END>(),
        .fft_start              = eventArray[3].getProfilingInfo<CL_PROFILING_COMMAND_START>(),
        // .fft_start              = eventArray[7].getProfilingInfo<CL_PROFILING_COMMAND_START>(),
        .fft_end                = eventArray[9].getProfilingInfo<CL_PROFILING_COMMAND_END>(),
        .read_start             = eventArray[10].getProfilingInfo<CL_PROFILING_COMMAND_START>(),
        .read_end               = eventArray[10].getProfilingInfo<CL_PROFILING_COMMAND_END>(),
    };
    return time;
}
