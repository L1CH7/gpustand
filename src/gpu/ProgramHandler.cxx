#include <ProgramHandler.hxx>
#include <error.hxx>

#include <fstream>
#include <cassert>

ProgramHandler::ProgramHandler( const std::shared_ptr< cl::Platform > & platform_, const std::shared_ptr< cl::Device > & device_ )
:   platform( platform_ ),
    device( device_ )
{}

cl_int
ProgramHandler::initializeDeviceWithKernelFile( const fs::path & kernel_path )
{
    cl_int error = CL_SUCCESS;

    std::ifstream kernel_file( kernel_path );
    if( !kernel_file.is_open() )
    {
        std::cout << "File not opened" << std::endl;
        assert( 0 );
    }
    std::string kernel_code(
        std::istreambuf_iterator< char >{ kernel_file },
        std::istreambuf_iterator< char >{}
    );
    kernel_file.close();

    cl::Program::Sources sources;
    sources.push_back( { kernel_code.c_str(), kernel_code.length() } );

    context = std::make_shared< cl::Context >( *device );
    program = std::make_shared< cl::Program >( *context, sources );
    try
    {
        error = program->build();
        std::stringstream info;

        info << "Build info:\n"
             << program->getBuildInfo< CL_PROGRAM_BUILD_LOG >( *device )
             << "\nEnd build info.\n";
        std::cout << info.str();
    }
    catch( cl::Error error )
    {
        std::stringstream msg;
        msg << "ERROR in " << __func__ << ": " << error.what() << ", "
                  << getErrorString( error.err() ) << std::endl;
        std::cout << error_str( msg.str() );
        throw error;
    }
    return error;
}


std::shared_ptr< ProgramHandler >
makeProgramHandler( size_t platform_num, size_t device_num, std::string & error )
{
    std::vector< cl::Platform > platforms;
    cl::Platform::get( &platforms );

    std::ostringstream msg;
    std::shared_ptr< cl::Platform > platform;
    std::shared_ptr< cl::Device > device;

    if( platforms.empty() || platforms.size() <= platform_num )
    {
        msg << "Platform [" << platform_num
            << "] not found. Total platforms: " << platforms.size() << "\n";
        error = msg.str();
        return nullptr;
    }
    else
    {
        platform = std::make_shared< cl::Platform >( platforms[ platform_num ] );
        std::vector< cl::Device > devices;
        platform->getDevices( CL_DEVICE_TYPE_ALL, &devices );
        if( devices.empty() || devices.size() <= device_num )
        {
            msg << "Device [" << device_num
                << "] not found. Total devices: " << devices.size() << "\n";
            error = msg.str();
            return nullptr;
        }
        else
            device = std::make_shared< cl::Device >( devices[ device_num ] );
    }
    return std::make_shared< ProgramHandler >( platform, device );
}