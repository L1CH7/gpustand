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
    )
    :   fft_instances_( computing_thread_num, nullptr ),
        pool_( 
            computing_thread_num,
            [ this, handler ]( std::size_t thread_id ){ 
                this->fft_instances_.at( thread_id ) = std::make_shared< FftCreator >( handler ); 
            }
        ),
        rdq_( rdq ),
        wdq_( wdq ),
        report_template_( report_template )
    {
    }

    void execute()
    {
        size_t task_index = 0;
        // can read data if:
        //      rdq is stopped but not empty
        //      rdq empty but not stopped (case of not waiting for ackquiring data)
        // resume: read if true: stop xor empty (or stop != empty)
        for( size_t i = 0; i < rdq_->size(); ++i )
        // while( rdq_->stopped() != rdq_->empty() )
        {
            FftReport report = report_template_;
            report.task_index = task_index++;

            auto task = [ this, report ]() mutable
            {
                // size_t thread_id = BS::this_thread::get_index().value_or(0);
                size_t thread_id = BS::this_thread::get_index().value_or(-1);
                if( thread_id > fft_instances_.size() )
                {
                    std::cerr << error_str("Invalid thread_id!\n");
                    return;
                }
                try
                {
                    auto data = rdq_->pop();
                    if( !data )
                    {
                        return; // acquired empty data!
                        this->pool_.purge();
                    }
                    FftParams params = data->params;
                    uint8_t polar = data->polar;
                    fs::path data_path = data->data_path;
                    fft_instances_[thread_id]->update( *data );
                    auto time_start = clock();//std::chrono::system_clock::to_time_t( std::chrono::system_clock::now() );

                    auto time = fft_instances_[thread_id]->compute();
                    auto out_array = fft_instances_[thread_id]->getFftResult();

                    auto time_end = clock();
                    
                    time.cpu_start_point = time_start;
                    time.cpu_end_point = time_end;

                    report.data_path = data_path;
                    report.out_array = std::move( out_array );
                    report.polar = polar;
                    report.params = params;
                    report.time = time;
                    wdq_->writeData( std::move( report ) );
                }
                catch( const cl::Error & e )
                {
                    std::stringstream ss;
                    ss << error_str( e.what() ) << std::endl;
                    ss << error_str( getErrorString( e.err() ) ) << std::endl;
                    ss << error_str("test#") << report.params.test_name << std::endl;
                    std::cerr << ss.str();
                    throw e;
                }
                catch( const std::exception & e )
                {
                    std::cerr << error_str(e.what()) <<std::endl;
                    throw e;
                }
            };
            pool_.detach_task( task );
        }

        if( rdq_->stopped() && rdq_->empty() )
            pool_.purge();

        pool_.wait();
    }
private:
    std::vector< std::shared_ptr < FftCreator > > fft_instances_;
    BS::light_thread_pool pool_;
    std::shared_ptr< ReadDataQueueInterface< FftData > > rdq_;
    std::shared_ptr< WriteDataQueueInterface< FftReport > > wdq_;
    const FftReport report_template_;
};

#endif // EXECUTION_POOL_H__
