// #ifdef ENABLE_DEBUG_COMPUTATIONS
// #   include <debug_computations.h> 
//     std::unique_ptr< fs::path > events_path = nullptr;
// #endif
#include <tests.h>

int main()
{
    const size_t platform_id = 3;
    const size_t device_id = 0;
    std::string error;
    ProgramHandler * handler = makeProgramHandler( platform_id, device_id, error );

    RunSingleTest( handler );
    // RunAllTests( handler );
    delete handler;

    return 0;
}