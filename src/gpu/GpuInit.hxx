#ifndef GPU_INIT_HXX__
#define GPU_INIT_HXX__

#include <string>
#include <CLDefs.hxx>
#include <ProgramHandler.hxx>
#include <DeviceIdentity.hxx>

// При инициализации модуля указываются смещения целевых 
// платформы и устройства в списках платформ и устройств; 
// списки выдаются clinfo -l
DeviceIdentity initGpuModule( std::shared_ptr< ProgramHandler > handler, const fs::path & kernel_path );

#endif // GPU_INIT_HXX__
