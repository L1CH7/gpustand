#include <GpuFourier.h>

#include <config.h>

#ifdef ENABLE_DEBUG_COMPUTATIONS
#   include <debug_computations.h>
#endif

FftInterface::FftInterface( ProgramHandler * handler, const FftParams & params, cl_int2 * dataArray )   
:   handler( std::move(handler) ),
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

void FftInterface::update( const FftParams & newParams, cl_int2 * newDataArray )
{
    params = std::move( newParams );
    dataArray = newDataArray;
}

void FftInterface::invariant()
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

FftCreator::FftCreator( ProgramHandler * handler, const FftParams & params, cl_int2 * dataArray )
{
    makeFftInterface( handler, params, dataArray );
}

FftCreator::~FftCreator()
{
    delete fft;
}

void FftCreator::update( const FftParams & newParams, cl_int2 * newDataArray )
{
    if( newParams.is_am == fft->params.is_am )
        fft->update( newParams, newDataArray );
    else
    {
        auto handler = fft->handler;
        delete fft;
        makeFftInterface( handler, newParams, newDataArray );
    }
}

TimeResult FftCreator::compute()
{
    return fft->compute();
}

cl_float2 * FftCreator::getFftResult() const
{
    return fft->outArray;
}

void FftCreator::makeFftInterface( ProgramHandler * handler, const FftParams & params, cl_int2 * dataArray )   
{
    if( params.is_am )
        fft = new AmFft( handler, params, dataArray );
    else
        fft = new FmFft( handler, params, dataArray );
}