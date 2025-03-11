#ifndef READ_DATA_QUEUE_H__
#define READ_DATA_QUEUE_H__

#include <AsyncDataQueueInterface.h>

template< typename Data_Tp >
class ReadDataQueueInterface : public AsyncDataQueueInterface< Data_Tp >
{
public:
    ReadDataQueueInterface() = delete;

    ReadDataQueueInterface( const size_t num_threads )
    :   AsyncDataQueueInterface< Data_Tp >( num_threads )
    {
    }

    ~ReadDataQueueInterface() = default;

    void startReading()
    {
        while( !this->stop_ )
        {
            auto task = [this]
            {
                std::optional< std::pair< Data_Tp, Data_Tp > > data = this->readData();
                if( !data.has_value() )
                {
                    this->stop(); // stop reading data
                    return;
                }
                
                auto [data0, data1] = data.value();
                this->push( std::move( data0 ) );
                this->push( std::move( data1 ) );
            };
            if( !this->stop_ )
                this->pool_.detach_task( task );
        };
    }

    // read 2 polars, needs to be override
    // readData MUST set stop_ flag when data ended!!
    virtual std::optional< std::pair< Data_Tp, Data_Tp > > readData() = 0;
};

#endif // READ_DATA_QUEUE_H__