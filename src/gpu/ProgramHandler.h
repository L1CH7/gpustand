#ifndef __PROGRAM_HANDLER_H
#define __PROGRAM_HANDLER_H


#include <fstream>
#include <vector>
#include <iostream>
#include <sstream>
#include <cassert>
#include <string>

#include <CLDefs.h>

struct ProgramHandler
{
    cl::Platform    * const platform;
    cl::Device      * const device;
    cl::Context     * context;
    cl::Program     * program;

    ProgramHandler( cl::Platform * plat, cl::Device * dev );

    ~ProgramHandler();

    cl_int
    initializeDeviceWithKernelFile( std::string file_name );
};

ProgramHandler * 
makeProgramHandler( size_t platform_num, size_t device_num, std::string & error );

#endif // __PROGRAM_HANDLER_H
