#ifndef JSON_HELPER_HXX__
#define JSON_HELPER_HXX__

#include <complex>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

// #include <types.hxx>

// Methods to convert std::complex from/to nlohmann::json
// It is necessary to write them in corresponding namespace - std
namespace std
{
    template< typename T >
    void 
    to_json( json & j, const std::complex< T > & c )
    {
        j[0] = c.real();
        j[1] = c.imag();
    }

    template< typename T >
    void 
    from_json( const json & j, std::complex< T > & c )
    {
        c = std::complex( j[0], j[1] );
    }

    // template< typename T >
    // void 
    // to_json( json & j, const std::unique_ptr< T > & ptr )
    // {
    //     if( ptr )
    //         j = *ptr;
    // }

    // template< typename T >
    // void 
    // from_json( const json & j, std::unique_ptr< T > & ptr )
    // {
    //     ptr = std::make_unique< T >( T( j ) );
    // }
}

#endif // JSON_HELPER_HXX__