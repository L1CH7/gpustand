#ifndef TESTS_PARALLEL_H__
#define TESTS_PARALLEL_H__

#include "tests.h"
#include <thread>

#include <BS_thread_pool.hpp>

void RunAllTestsParallel( FftCreator & fft, const fs::path & testcases_dir );

void RunAllTestsParallelV2( std::shared_ptr< ProgramHandler > handler, const fs::path & testcases_dir );

void RunAllTestsParallelV3( std::shared_ptr< ProgramHandler > handler, const fs::path & testcases_dir );
void RunAllTestsParallelV4( std::shared_ptr< ProgramHandler > handler, const fs::path & testcases_dir );

#endif // TESTS_PARALLEL_H__