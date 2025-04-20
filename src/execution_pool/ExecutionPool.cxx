#include <ExecutionPool.hxx>

#include <algorithm> // std::partial_sort_copy

ExecutionPool::ExecutionPool(  
    std::shared_ptr< ProgramHandler > handler, 
    std::shared_ptr< ReadDataQueueInterface< FftData > > rdq, 
    std::shared_ptr< WriteDataQueueInterface< FftReport > > wdq, 
    const FftReport report_template,
    const size_t computing_thread_num
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


void ExecutionPool::execute()
{
    size_t task_index = 0;
    for( size_t i = 0; i < rdq_->size(); ++i )
    // can read data if:
    //      rdq is stopped but not empty
    //      rdq empty but not stopped (case of not waiting for ackquiring data)
    // resume: read if true: stop xor empty (or stop != empty)
    // while( rdq_->stopped() != rdq_->empty() )
    // while( rdq_->running() )
    {
        // std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
        FftReport report = report_template_;
        report.task_index = task_index++;

        auto task = [ this, report ]() mutable
        {
            size_t thread_id = BS::this_thread::get_index().value_or( -1 );
            if( thread_id > fft_instances_.size() )
            {
                std::cerr << error_str("Invalid thread_id!\n");
                return;
            }
            // report.is_data_valid = false; // in case of error returns false by default
            try
            {
                auto data = rdq_->pop();
                if( !data )
                {
                    return; // acquired empty data!
                    this->pool_.purge();
                }
                /**
                 * make this member local. this shouldn`t read info that many times
                 * report.device_name = handler_->device->getInfo<CL_DEVICE_NAME>();
                 */
                report.params = data->params;
                report.polar = data->polar;
                report.data_path = data->data_path;
                report.src_verif = std::move( data->verification );
                fft_instances_[thread_id]->update( *data );
                
                report.time = fft_instances_[thread_id]->compute();
                report.out_array = fft_instances_[thread_id]->getFftResult();
                report.out_verif = makeDataHashWrapper( report.out_array, report.params, 3, .1 );
                report.accurate_result = report.src_verif == report.out_verif;
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
            wdq_->writeData( std::move( report ) );
        };
        pool_.detach_task( task );
    }

    if( rdq_->stopped() && rdq_->empty() )
        pool_.purge();

    pool_.wait();
}
