#ifndef READ_DATA_QUEUE_H__
#define READ_DATA_QUEUE_H__

#include <AsyncDataQueueInterface.h>
// #include <DataSourceInterface.h>

/**
 * Кадр ethernet выыглядит так: коммутатор не считывает кадр целиком, а лишь первые 6 байт -- адрес получателя.
 * Аналогично в случае наших данных: в заголовке лежат FftParams, в остальной части находится строб.
 * Такое представление позволит более гибко обрабатывать и фильтровать данные еще на этапе считывания, и в последствии
 * позволит сделать приоритеты для очереди выполняемых задач
 */

/**
 * Data_Tp - data type for reading
 * Filter_Tp - used to filter which incoming packages we need to acquire
 */
// template< typename Data_Tp, typename Filter_Tp >
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

    virtual void startReading() = 0;
    // void startReading()
    // {
    //     while( !this->stop_reading_.load( std::memory_order_relaxed ) )
    //     {
    //         // std::atomic_thread_fence(std::memory_order_acquire);
    //         // std::this_thread::yield();
    //         // std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
    //         // if( this->pool_.get_tasks_queued() > 3 * this->pool_.get_thread_count() )
    //         //     this->pool_.wait();


    //         // auto filtered = this->filterData();
    //         // if( filtered != std::nullopt )
    //         //     continue;   // skip data
            
    //         this->readDataPrepare();
    //         if( !this->stop_reading_.load( std::memory_order_relaxed ) )
    //             break; // stop reading

    //         auto task = [this]
    //         {
    //             // std::optional< std::pair< Data_Tp, Data_Tp > > data = this->data_source_.getData();
    //             std::optional< std::pair< Data_Tp, Data_Tp > > data = this->readData();
    //             // if( !data.has_value() )
    //             // {
    //             //     this->stop(); // stop reading data
    //             //     return;
    //             // }
    //             if( data.has_value() )
    //             {
    //                 auto [data0, data1] = std::move( data.value() );
    //                 this->push( std::move( data0 ) );
    //                 this->push( std::move( data1 ) );
    //             }
    //             else
    //             {
    //                 // std::cout << warn_str("no value!!\n");
    //                 return;
    //             }
    //         };
    //         this->pool_.detach_task( task );
    //         // else{
    //         //     std::cout << warn_str("queue stopped!");
    //         //     this->pool_.purge();
    //         // }
    //     }
    // }

protected:
    /**
     * used to check if reached stop condition
     */
    // virtual bool checkStopCondition() = 0;

    /**
     * used to filter data for reading
     */
    // virtual std::optional< Data_Tp > filterData() = 0;
    // bool filterData( const Data_Tp & data );

    /**
     * read header of data. Must stop if reached end
     */
    // virtual void readDataPrepare() = 0;

    /**
     * read 2 polars, needs to be override
     * getData MUST set stop_ flag when data ended!!
     */
    // virtual std::optional< std::pair< Data_Tp, Data_Tp > > readData() = 0;

    /**
     * used to filter data for reading
     */
    // Filter_Tp read_filter_{};
//     DataSourceInterface< FftData > data_source_;
};

#endif // READ_DATA_QUEUE_H__