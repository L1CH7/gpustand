#ifndef WRITE_DATA_QUEUE_H__
#define WRITE_DATA_QUEUE_H__

#include <AsyncDataQueueInterface.h>

template< typename Data_Tp >
class WriteDataQueueInterface : public AsyncDataQueueInterface< Data_Tp >
{
public:
    WriteDataQueueInterface() = delete;

    WriteDataQueueInterface( const size_t num_threads )
    :   AsyncDataQueueInterface< Data_Tp >( num_threads )
    {
    }

    ~WriteDataQueueInterface() = default;

    virtual void writeData( Data_Tp && data ) = 0;
};

#endif // WRITE_DATA_QUEUE_H__