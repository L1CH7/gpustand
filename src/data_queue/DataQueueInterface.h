#ifndef DATA_QUEUE_INTERFACE_H__
#define DATA_QUEUE_INTERFACE_H__

#include <queue>
#include <memory>
#include <mutex>
// #include <thread>



template< typename Data_Tp >
class DataQueueInterface
{
public:
    DataQueueInterface()
    {
        // collectData();
    }

    ~DataQueueInterface() = default;

    void push( Data_Tp && data )
    {
        std::scoped_lock( queue_mutex_ );
        q_.push( std::make_unique< Data_Tp >( std::move( data ) ) );
    }

    std::unique_ptr< Data_Tp > pop()
    {
        std::scoped_lock( queue_mutex_ );
        auto popped{ std::move( q_.front() ) };
        // auto popped = q_.front().reset();
        q_.pop();
        return popped;
        // return std::move( popped );
    }

    bool empty()
    {
        std::scoped_lock( queue_mutex_ );
        return q_.empty();
    }

    size_t size()
    {
        std::scoped_lock( queue_mutex_ );
        return q_.size();
    }

protected:
    virtual void collectData() = 0;

    std::queue< std::unique_ptr< Data_Tp > > q_;
    std::mutex queue_mutex_{};
};

#endif // DATA_QUEUE_INTERFACE_H__