#ifndef READ_DATA_QUEUE_HXX__
#define READ_DATA_QUEUE_HXX__

#include <AsyncDataQueueInterface.hxx>
// #include <DataSourceInterface.hxx>

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
};

#endif // READ_DATA_QUEUE_HXX__