#include <GpuFourier.h>

#include <config.h>

#ifdef ENABLE_DEBUG_COMPUTATIONS
#   include <debug_computations.h>
#endif

FftInterface::FftInterface( std::shared_ptr< ProgramHandler > handler, FftData & data )   
:   handler_( handler ),
    params_( data.params ),
    mseq_( std::move( data.mseq ) ),
    data_array_( std::move( data.data_array ) ),
    out_array_( params_.nl * params_.kgd * params_.kgrs, {0, 0} )
{
    invariant();
}


FftInterface::FftInterface( std::shared_ptr< ProgramHandler > handler )   
:   handler_( handler ),
    params_(),
    mseq_(),
    data_array_(),
    out_array_()
{
}


void
FftInterface::update( FftData & new_data )
{
    params_ = new_data.params;
    mseq_ = std::move( new_data.mseq );
    data_array_ = std::move( new_data.data_array );
    out_array_ = std::vector< std::complex< float > >( params_.nl * params_.kgd * params_.kgrs, {0, 0} );
    invariant();
}

void
FftInterface::invariant()
{
    if( handler_ == nullptr )
    {
        std::cerr << error_str( "No OpenCL handler found" );
        assert(0);
    }
    if( params_.nl < 1 || params_.kgd < 1 || params_.kgrs < 1 )
    {
        std::cerr << error_str( "Incorrect params" );
        assert(0);
    }
    if( data_array_.size() < params_.samples_num || ( !params_.is_am && mseq_.size() < params_.true_nihs ) )
    {   
        std::stringstream ss;
        ss << error_str( "Not enough data: test#" ) << params_.test_name
            << focus_str("\nsamples_num = ") << params_.samples_num << focus_str("; array_size = ") << data_array_.size();
        if( !params_.is_am )
            ss << focus_str("\true_nihs = ") << params_.true_nihs << focus_str("; mseq_size = ") << mseq_.size() << std::endl;
        std::cerr << ss.str();
        assert(0);
    }
}


class DummyFft: public FftInterface
{
public:
    DummyFft( std::shared_ptr< ProgramHandler > handler )
    :   FftInterface( handler )
    {
        // flag identificates invalid fft interface
        ready = false;
        std::cout << "Dummy FFT instance c-tor!\n";
    }
    
    ~DummyFft() = default;

    TimeResult compute()
    {
        return {};
    }
};

FftCreator::FftCreator( std::shared_ptr< ProgramHandler > handler )
{
    fft_ = std::make_unique< DummyFft >( handler );
}

FftCreator::FftCreator( std::shared_ptr< ProgramHandler > handler, FftData & data )
{
    makeFftInterface( handler, data );
}

bool 
FftCreator::hasFftInterface()
{
    return fft_ != nullptr;
}

void
FftCreator::update( FftData & new_data )
{
    if( fft_->ready && new_data.params.is_am == fft_->params_.is_am )
        fft_->update( new_data );
    else
        makeFftInterface( fft_->handler_, new_data );
}

TimeResult
FftCreator::compute()
{
    return fft_->compute();
}

std::vector< std::complex< float > >
FftCreator::getFftResult()
{
    return std::move( fft_->out_array_ );
}

void
FftCreator::makeFftInterface( std::shared_ptr< ProgramHandler > handler, FftData & data )
{
    if( fft_ )
        fft_.reset(); // deletes pointer

    if( data.params.is_am )
        fft_ = std::make_unique< AmFft >( handler, data );
    else
    {
        uint32_t N = 1 << data.params.log2N;
        uint64_t mid_buffer_size = data.params.nl * data.params.kgrs * N * sizeof(cl_float2);
        uint64_t max_buffer_size = handler->device->getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>();
        data.mseq.resize( N, 0 );

        if (mid_buffer_size > max_buffer_size)
            fft_ = std::make_unique< FmFftSepNl >( handler, data );
        else
            fft_ = std::make_unique< FmFft >( handler, data );
    }
}