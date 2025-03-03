#ifndef DEBUG_CUMPUTATIONS_H__
#define DEBUG_CUMPUTATIONS_H__

#ifdef ENABLE_DEBUG_COMPUTATIONS

#include <config.h>
#include <error.h>
#include <CLDefs.h>
#include <IoJson.h>

// global variable to get path for printing events
extern std::unique_ptr< fs::path > events_path;// = nullptr;

void changeEventsPath( const fs::path & path )
{
    auto new_path = std::make_unique< fs::path >( path );
    events_path = std::move( new_path );
}

template< typename Buffer_Tp, typename Vector_Tp >
void
writeBufferToJsonFile( const fs::path & filepath, cl::CommandQueue & queue, cl::Buffer & buffer, size_t buffer_size )
{
    assert( sizeof( Buffer_Tp ) == sizeof( Vector_Tp ) && "Equality of type sizes" );

    std::vector< Vector_Tp > out( buffer_size );
    auto t_out = reinterpret_cast< Buffer_Tp * >( out.data() );
    queue.enqueueReadBuffer(
        buffer, CL_TRUE, 0,
        buffer_size * sizeof( Buffer_Tp ), t_out,
        NULL, NULL );     

    if( events_path == nullptr )   
        std::cout << error_str( "Path for printing buffer not initialized!\n" );
    else
        writeVectorToJsonFile( (*events_path) / filepath, out );
}

template< typename Data_Tp, typename Vector_Tp >
void
writePtrArrayToJsonFile( const fs::path & filepath, Data_Tp * _array, size_t _size )
{
    assert( sizeof( Data_Tp ) == sizeof( Vector_Tp ) && "Equality of type sizes" );

    Vector_Tp * _begin = reinterpret_cast< Vector_Tp * >( _array );
    std::vector< Vector_Tp > out( _begin, _begin + _size );         

    if( events_path == nullptr )   
        std::cout << error_str( "Path for printing buffer not initialized!\n" );
    else
        writeVectorToJsonFile( (*events_path) / filepath, out );
}
#endif

#endif // DEBUG_CUMPUTATIONS_H__