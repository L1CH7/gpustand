#ifndef DEBUG_CUMPUTATIONS_H__
#define DEBUG_CUMPUTATIONS_H__
#include <config.h>

#ifdef ENABLE_DEBUG_COMPUTATIONS

#include <IoJson.h>
    template< typename Buffer_Tp, typename Vector_Tp >
    void
    writeBufferToJsonFile( const std::string & filename, cl::CommandQueue & queue, cl::Buffer & buffer, size_t buffer_size )
    {
        assert( sizeof( Buffer_Tp ) == sizeof( Vector_Tp ) && "Equality of type sizes" );

        std::vector< Vector_Tp > out( buffer_size );
        auto t_out = reinterpret_cast< Buffer_Tp * >( out.data() );
        queue.enqueueReadBuffer(
            buffer, CL_TRUE, 0,
            buffer_size * sizeof( Buffer_Tp ), t_out,
            NULL, NULL );                                            
        writeVectorToJsonFile( RESULT_DIR+filename, out );
    }

    template< typename Data_Tp, typename Vector_Tp >
    void
    writePtrArrayToJsonFile( const std::string & filename, Data_Tp * _array, size_t _size )
    {
        assert( sizeof( Data_Tp ) == sizeof( Vector_Tp ) && "Equality of type sizes" );

        Vector_Tp * _begin = reinterpret_cast< Vector_Tp * >( _array );
        std::vector< Vector_Tp > out( _begin, _begin + _size );                                           

        writeVectorToJsonFile( RESULT_DIR+filename, out );
    }
#endif

#endif // DEBUG_CUMPUTATIONS_H__