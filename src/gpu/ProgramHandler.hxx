#ifndef PROGRAM_HANDLER_HXX__
#define PROGRAM_HANDLER_HXX__

#include <CLDefs.hxx>
#include <memory>
#include <filesystem>
namespace fs = std::filesystem;

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

#endif // PROGRAM_HANDLER_HXX__
