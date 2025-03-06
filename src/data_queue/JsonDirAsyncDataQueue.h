#ifndef JSON_DIR_DATA_QUEUE_H__
#define JSON_DIR_DATA_QUEUE_H__

#include "AsyncDataQueueInterface.h"

#define RANDOMIZE_TESTS
#ifdef RANDOMIZE_TESTS
#   include <algorithm>
#   include <random>
// #   define TEST_MULTIPLIER 3
#endif

#define PARALLELL_READING

// #define WAIT_FOR_DIR_COUNT 20


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

class JsonDirAsyncDataQueue : public AsyncDataQueueInterface< FftData, PathsTemplate >
{
public:
    JsonDirAsyncDataQueue() = delete;

    explicit JsonDirAsyncDataQueue( const fs::path & directory, const PathsTemplate p_template, const size_t num_threads );

    inline void wait(){ pool_.wait(); }

    bool finish(){ return finish_; }

private:
    void collectData();

    const PathsTemplate paths_;
    const fs::path directory_;
    bool finish_{ false };
};

#endif // JSON_DIR_DATA_QUEUE_H__