#ifndef ASYNC_DATA_QUEUE_INTERFACE_H__
#define ASYNC_DATA_QUEUE_INTERFACE_H__

#include <DataQueueInterface.h>
#include <thread>
#include <mutex>
#include <BS_thread_pool.hpp>


template< typename Data_Tp, typename Get_Tp >
class AsyncDataQueueInterface : public DataQueueInterface< Data_Tp >
{
public:
    AsyncDataQueueInterface() = delete;

    AsyncDataQueueInterface( const size_t num_threads )
    :   pool_( num_threads )
    {
    }

    ~AsyncDataQueueInterface() = default;

    void push( Data_Tp && data ) override
    {
        std::scoped_lock( queue_mutex_ );
        q_.push( std::make_unique< Data_Tp >( std::move( data ) ) );
    }

    std::unique_ptr< Data_Tp > pop() override
    {
        std::scoped_lock( queue_mutex_ );
        auto popped{ std::move( q_.front() ) };
        q_.pop();
        return popped;
    }

    bool empty() override
    {
        std::scoped_lock( queue_mutex_ );
        return q_.empty();
    }

    size_t size() override
    {
        std::scoped_lock( queue_mutex_ );
        return q_.size();
    }

    std::unique_ptr< Data_Tp > popOrWait()
    {
        if( empty() )
            return nullptr;

        if( futures_.ready_count() == 0 )
            futures_.front().wait();
        
        return pop();

        // pool_.
        // return nullptr;
    }

protected:
    void collectData()
    {
        Get_Tp got;
        std::future< void > main_cycle = []
        {
            while( got = getOneData() )
            {
                auto task = []
                {

                };
                futures_.push_back( pool_.submit_task( task ) );
            }
        }
    }

    virtual Get_Tp getOneData() = 0;

    mutable std::mutex queue_mutex_{};

    BS::light_thread_pool pool_;

    BS::multi_future< Data_Tp > futures_;

    bool finish_;
};

#endif // ASYNC_DATA_QUEUE_INTERFACE_H__