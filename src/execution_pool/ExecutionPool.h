#ifndef EXECUTION_POOL_H__
#define EXECUTION_POOL_H__

#include <BS_thread_pool.hpp>
#include <ReadDataQueueInterface.h>
#include <WriteDataQueueInterface.h>
#include <memory> // shared_ptr
#include <types.h> // *
#include <ProgramHandler.h> // ProgramHandler
#include <GpuFourier.h> // FftCreator

class ExecutionPool
{
public:
    ExecutionPool(  
        std::shared_ptr< ProgramHandler > handler, 
        std::shared_ptr< ReadDataQueueInterface< FftData > > rdq, 
        std::shared_ptr< WriteDataQueueInterface< FftReport > > wdq, 
        const FftReport report_template,
        const size_t computing_thread_num = 1 
    );

    void execute();

    // bool verificate( const std::vector< std::complex< float > > & data, const FftParams params );
    
private:
    std::vector< std::shared_ptr < FftCreator > > fft_instances_;
    BS::light_thread_pool pool_;
    std::shared_ptr< ReadDataQueueInterface< FftData > > rdq_;
    std::shared_ptr< WriteDataQueueInterface< FftReport > > wdq_;
    const FftReport report_template_;
};

#endif // EXECUTION_POOL_H__
