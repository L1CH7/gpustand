#ifndef DATA_QUEUE_INTERFACE_H__
#define DATA_QUEUE_INTERFACE_H__

#include <queue>

template< typename Data_Tp >
class DataQueueInterface
{
public:
    virtual void push( Data_Tp && data ) = 0;
    virtual std::unique_ptr< Data_Tp > pop() = 0;
    virtual bool empty() const = 0;
    virtual size_t size() const = 0;

protected:
    std::queue< std::unique_ptr< Data_Tp > > q_;
};

#endif // DATA_QUEUE_INTERFACE_H__