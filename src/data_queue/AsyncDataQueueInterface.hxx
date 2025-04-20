#ifndef ASYNC_DATA_QUEUE_INTERFACE_HXX__
#define ASYNC_DATA_QUEUE_INTERFACE_HXX__

#include <DataQueueInterface.hxx>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <BS_thread_pool.hpp>

template< typename Data_Tp >
class AsyncDataQueueInterface : public DataQueueInterface< Data_Tp >
{
public:
    AsyncDataQueueInterface() = delete;

    AsyncDataQueueInterface( const size_t num_threads )
    :   pool_( num_threads ),
        stop_( false ),
        stop_reading_( false )
    {
    }

    ~AsyncDataQueueInterface() = default;

    void push( Data_Tp && data ) override
    {
        std::scoped_lock< std::mutex > lock( queue_mutex_ );
        this->q_.push( std::make_unique< Data_Tp >( std::move( data ) ) );
        cv_.notify_one(); // Уведомление о поступлении новых данных
    }

    std::unique_ptr< Data_Tp > pop() override
    {
        std::unique_lock< std::mutex > lock( queue_mutex_ );
        cv_.wait( lock, [this]{ return !this->q_.empty() || this->stop_; } );
        
        if( this->stop_ && this->q_.empty() )
            // throw std::runtime_error( error_str( "Queue is empty and stopped" ) );
            return nullptr;
        
        std::unique_ptr< Data_Tp > data = std::move( this->q_.front() );
        this->q_.pop();
        return data;
    }

    /**
     * get_tasks_total() == get_tasks_queued() + get_tasks_running()
     * returns true if queue has tasks queued or running
     */
    bool running() const
    {
        return pool_.get_tasks_total() != 0;
    }

    bool empty() const override
    {
        std::scoped_lock< std::mutex > lock( queue_mutex_ );
        return this->q_.empty();
    }

    bool stopped() const
    {
        // std::scoped_lock< std::mutex > lock( queue_mutex_ );
        // stop_ = pool_.get_tasks_running() == 0;
        return stop_;
    }

    size_t size() const override
    {
        std::scoped_lock< std::mutex > lock( queue_mutex_ );
        return this->q_.size();
    }

    void stop()
    {
        std::scoped_lock< std::mutex > lock( queue_mutex_ );
        stop_ = true;
        cv_.notify_all(); // Уведомление о завершении приема данных
    }

    void resume()
    {
        std::scoped_lock< std::mutex > lock( queue_mutex_ );
        stop_ = false;
        cv_.notify_all(); // Уведомление о завершении приема данных
    }

    void wait()
    {
        pool_.wait();
    }

protected:
    mutable std::mutex queue_mutex_{};

    std::condition_variable cv_{};

    BS::light_thread_pool pool_;

    std::atomic< bool > stop_reading_;

    bool stop_;
};

#endif // ASYNC_DATA_QUEUE_INTERFACE_HXX__