#ifndef PROGRAM_HANDLER_H__
#define PROGRAM_HANDLER_H__


#include <fstream>
#include <vector>
#include <iostream>
#include <sstream>
#include <cassert>
#include <string>

#include <memory>

#include <types.h>
#include <CLDefs.h>

/** Hint for const:
 * 
 * shared_ptr<T> p;             ---> T * p;                                    : nothing is const
 * const shared_ptr<T> p;       ---> T * const p;                              : p is const
 * shared_ptr<const T> p;       ---> const T * p;       <=> T const * p;       : *p is const
 * const shared_ptr<const T> p; ---> const T * const p; <=> T const * const p; : p and *p are const.
 * 
 */

struct ProgramHandler
{
    const std::shared_ptr< cl::Platform >   platform;
    const std::shared_ptr< cl::Device >     device;
    std::shared_ptr< cl::Context >          context;
    std::shared_ptr< cl::Program >          program;
    // cl::Platform    * const platform;
    // cl::Device      * const device;
    // cl::Context     * context;
    // cl::Program     * program;

    ProgramHandler( const std::shared_ptr< cl::Platform > & platform_, const std::shared_ptr< cl::Device > & device_ );

    ~ProgramHandler() = default;

    cl_int
    initializeDeviceWithKernelFile( const fs::path & kernel_path );
};

std::shared_ptr< ProgramHandler >
makeProgramHandler( size_t platform_num, size_t device_num, std::string & error );

#endif // PROGRAM_HANDLER_H__
