#ifndef JSON_DIR_DATA_QUEUE_H__
#define JSON_DIR_DATA_QUEUE_H__

#include "DataQueueInterface.h"
#include <thread>

#define RANDOMIZE_TESTS
#ifdef RANDOMIZE_TESTS
#   include <algorithm>
#   include <random>
#   define TEST_MULTIPLIER 3
#endif
#define PARALLELL_READING


#include <config.h>
#include <types.h>
#include <IoJson.h>
#include <BS_thread_pool.hpp>


// template for parameters and data locations
struct PathsTemplate
{
    fs::path params_path;
    fs::path mseq_path;
    fs::path data_path;
};

class JsonDirDataQueue : public DataQueueInterface< FftData >
{
public:
    JsonDirDataQueue() = delete;

    explicit JsonDirDataQueue( const fs::path & directory, const PathsTemplate p_template, const size_t num_threads );

private:
    void collectData();

    const PathsTemplate paths_;
    const fs::path directory_;
    BS::light_thread_pool pool_;
};

#endif // JSON_DIR_DATA_QUEUE_H__