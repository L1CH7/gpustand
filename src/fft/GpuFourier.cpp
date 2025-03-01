#include <GpuFourier.h>

#include <config.h>

#ifdef ENABLE_DEBUG_COMPUTATIONS
#   include <debug_computations.h>
#endif

FftInterface::FftInterface( std::shared_ptr< ProgramHandler > handler, const FftParams & params, cl_int2 * dataArray )   
:   handler( handler ),
    params( params ),
    dataArray( dataArray ),
    outArray( new cl_float2[params.nl * params.kgd * params.kgrs] )
{
    invariant();
}

FftInterface::~FftInterface()
{
    if( outArray )
        delete outArray;
}

FftInterface::FftInterface( std::shared_ptr< ProgramHandler > handler )   
:   handler( handler ),
    params( {} ),
    dataArray( nullptr ),
    outArray( nullptr )
{
}

void FftInterface::setParams( const FftParams & newParams )
{
    params = newParams;
    if( outArray )
        delete outArray;
    outArray = new cl_float2[params.nl * params.kgd * params.kgrs];
}

void FftInterface::setDataArray( cl_int2 * newDataArray )
{
    dataArray = newDataArray;
    // Assumed that params are defined
    invariant();
}

void
FftInterface::update( const FftParams & newParams, cl_int2 * newDataArray )
{
    setParams( newParams );
    setDataArray( newDataArray );
}

void
FftInterface::invariant()
{
    if( handler == nullptr )
    {
        PRINT_ERROR("No OpenCL handler found");
        assert(0);
    }
    if( params.nl < 1 || params.kgd < 1 || params.kgrs < 1 )
    {
        PRINT_ERROR("Incorrect params");
        assert(0);
    }
    // if( dataArray.size() < params.samples_num || ( !params.is_am && params.mseq.size() < params.true_nihs ) )
    if( ( !params.is_am && params.mseq.size() < params.true_nihs ) )
    {
        PRINT_ERROR("Not enough data");
        assert(0);
    }
}

FftCreator::FftCreator( std::shared_ptr< ProgramHandler > handler, const FftParams & params, cl_int2 * dataArray )
{
    makeFftInterface( handler, params, dataArray );
}

bool 
FftCreator::hasFftInterface()
{
    return fft != nullptr;
}

void
FftCreator::update( const FftParams & newParams, cl_int2 * newDataArray )
{
    if( newParams.is_am == fft->params.is_am )
        fft->update( newParams, newDataArray );
    else
    {
        auto handler = fft->handler;
        makeFftInterface( handler, newParams, newDataArray );
    }
}

TimeResult
FftCreator::compute()
{
    return fft->compute();
}

cl_float2 *
FftCreator::getFftResult() const
{
    return fft->outArray;
}

void
FftCreator::makeFftInterface( std::shared_ptr< ProgramHandler > handler, const FftParams & params, cl_int2 * dataArray )   
{
    if( params.is_am )
        fft = std::make_shared< AmFft >( handler, params, dataArray );
    else
    {
        uint32_t N = 1 << params.log2N;
        uint64_t midBufferSize = params.nl * params.kgrs * N * sizeof(cl_float2);
        uint64_t maxBufferSize = handler->device->getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>();
        
        if (midBufferSize > maxBufferSize)
            fft = std::make_shared< FmFftSepNl >( handler, params, dataArray );
        else
            fft = std::make_shared< FmFft >( handler, params, dataArray );
    }
}