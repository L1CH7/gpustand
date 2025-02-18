#include "ProgramHandler.h"
// #include <CLDefs.h>

ProgramHandler::ProgramHandler( cl::Platform * plat, cl::Device * dev )
:   platform( plat ),
    device( dev )
{}

ProgramHandler::~ProgramHandler()
{
    delete( platform );
    delete( device );
    delete( context );
    delete( program );
}

cl_int
ProgramHandler::initializeDeviceWithKernelFile( std::string file_name )
{
    cl_int error = CL_SUCCESS;

    std::ifstream kernel_file( file_name );
    if( !kernel_file.is_open() )
    {
        std::cout << "File not opened" << std::endl;
        assert( kernel_file.is_open() );
    }
    std::string kernel_code(
        std::istreambuf_iterator< char >{ kernel_file },
        std::istreambuf_iterator< char >{}
    );
    kernel_file.close();

    cl::Program::Sources sources;
    sources.push_back({ kernel_code.c_str(), kernel_code.length() });

    context = new cl::Context( *device );
    program = new cl::Program( *context, sources );
    try
    {
        error = program->build();
        std::cout << "Build info:\n"
                    << program->getBuildInfo< CL_PROGRAM_BUILD_LOG >( *device )
                    << "\nEnd build info.\n";
    }
    catch( cl::Error error )
    {
        std::cerr << "ERROR in " << __func__ << ": " << error.what() << ", "
                << getErrorString( error.err() ) << std::endl;
        throw error;
    }
    return error;
}


ProgramHandler *
makeProgramHandler( size_t platform_num, size_t device_num, std::string & error )
{
    std::vector< cl::Platform > platforms;
    cl::Platform::get( &platforms );

    std::ostringstream msg;
    cl::Platform * platform;
    cl::Device * device;

    if( platforms.empty() || platforms.size() <= platform_num )
    {
        msg << "Platform [" << platform_num
            << "] not found. Total platforms: " << platforms.size() << "\n";
        error = msg.str();
        return nullptr;
    }
    else
    {
        platform = new cl::Platform( platforms[ platform_num ] );
        std::vector< cl::Device > devices;
        platform->getDevices( CL_DEVICE_TYPE_ALL, &devices );
        if( devices.empty() || devices.size() <= device_num )
        {
            msg << "Device [" << device_num
                << "] not found. Total devices: " << devices.size() << "\n";
            error = msg.str();
            delete platform; // cleanup
            return nullptr;
        }
        else
            device = new cl::Device( devices[ device_num ] );
    }
    return new ProgramHandler( platform, device );
}