#include "GpuInit.h"
#include <error.h>

DeviceIdentity initGpuModule(
    ProgramHandler * handler,
    const std::string & kernel_file
)
{
    std::cout << "--> " << __func__ << "("
         << "\"" << kernel_file << "\")"
         << std::endl;
    try
    {
        if( handler )
        {
            handler->initializeDeviceWithKernelFile(kernel_file);
            return DeviceIdentity {
                handler->device->getInfo<CL_DEVICE_NAME>(),
                handler->device->getInfo<CL_DEVICE_VERSION>(),
                handler->platform->getInfo<CL_PLATFORM_NAME>(),
                handler->platform->getInfo<CL_PLATFORM_VERSION>()
            };
        }
        PRINT_ERROR("No handler created");
        return{};
    }
    catch (cl::Error err) {
        std::string err_str = "No handler created" + std::string(err.what());
        PRINT_ERROR(err_str);
        std::cerr << "No handler created: " << err.what() << std::endl;
        throw err;
    }
}