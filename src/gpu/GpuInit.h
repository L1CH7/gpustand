#ifndef GPU_INIT_H__
#define GPU_INIT_H__

#include <string>
#include <types.h>
#include <CLDefs.h>
#include <ProgramHandler.h>

// При инициализации модуля указываются смещения целевых 
// платформы и устройства в списках платформ и устройств; 
// списки выдаются clinfo -l
DeviceIdentity initGpuModule( std::shared_ptr< ProgramHandler > handler, const fs::path & kernel_path );

#endif // GPU_INIT_H__
