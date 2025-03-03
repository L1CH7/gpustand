#include "GpuInit.h"
#include <error.h>

DeviceIdentity initGpuModule( std::shared_ptr< ProgramHandler > handler, const fs::path & kernel_path )
{
    std::cout << "--> " << __func__ << "("
         << "\"" << kernel_path.native() << "\")"
         << std::endl;
    try
    {
        if( handler )
        {
            handler->initializeDeviceWithKernelFile( kernel_path );
            return DeviceIdentity{
                .device_name = handler->device->getInfo<CL_DEVICE_NAME>(),
                .device_version = handler->device->getInfo<CL_DEVICE_VERSION>(),
                .platform_name = handler->platform->getInfo<CL_PLATFORM_NAME>(),
                .platform_version = handler->platform->getInfo<CL_PLATFORM_VERSION>()
            };
        }
        std::cerr << error_str( "No handler created" );
        return{};
    }
    catch (cl::Error err) {
        std::cerr << error_str( "No handler created" + std::string(err.what()) );
        std::cerr << "No handler created: " << err.what() << std::endl;
        throw err;
    }
}