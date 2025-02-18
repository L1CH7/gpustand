#ifndef GPU_INIT_H__
#define GPU_INIT_H__

#include <string>
#include <types.h>
#include <CLDefs.h>
#include <ProgramHandler.h>

// При инициализации модуля указываются смещения целевых 
// платформы и устройства в списках платформ и устройств; 
// списки выдаются clinfo -l
DeviceIdentity initGpuModule(
    ProgramHandler * handler,
    const std::string & kernel_file);

#endif // GPU_INIT_H__
