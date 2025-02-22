#include <tests.h>

int main()
{
    const size_t platform_id = 3;
    const size_t device_id = 0;
    std::string error;
    ProgramHandler * handler = makeProgramHandler( platform_id, device_id, error );

    RunTestsSingleThread( handler );
    delete handler;

    return 0;
}