#ifndef JSON_WRITE_DATA_QUEUE_HXX__
#define JSON_WRITE_DATA_QUEUE_HXX__

#include <WriteDataQueueInterface.hxx>
#include <FftReport.hxx>
#include <IoJson.hxx>

class JsonWriteDataQueue : public WriteDataQueueInterface< FftReport >
{
public:
    JsonWriteDataQueue() = delete;

    JsonWriteDataQueue( const size_t num_threads )
    :   WriteDataQueueInterface< FftReport >( num_threads )
    {
    }

    void writeData( FftReport && r ) override
    {
        std::string test_name = r.data_path.filename().native();

        std::stringstream ss_result;
        ss_result << test_name << "_result_polar" << std::to_string( r.polar ) << "_" << r.task_index << ".json";
        std::string result_name = ss_result.str();

        std::stringstream ss_report;
        ss_report << test_name << "_polar" << std::to_string( r.polar ) << "_" << r.task_index << ".json";
        std::string report_name = ss_report.str();
        
        push( std::move( r ) );
        auto task = [this, result_name, report_name]
        {
            auto r = pop();
            
            std::stringstream info;
            info << "writing: " << report_name << std::endl;
            std::cout << info.str();
            IoJson::writeReport( r->report_dir / report_name, *r );
            // we dont need to write results
            // IoJson::writeFftResult( r->result_dir / result_name, r->out_array, r->polar, r->params );
        };
        pool_.detach_task( task );
    }
};

#endif // JSON_WRITE_DATA_QUEUE_HXX__