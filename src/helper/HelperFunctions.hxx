#ifndef HELPER_FUNCTIONS_HXX__
#define HELPER_FUNCTIONS_HXX__

// #include <vector>
// #include <complex>
// #include <types.hxx>
#include <algorithm>

namespace Helper
{
    // template< typename T >

    /**
     * splits array into NL parts
     * from every part of it gets N greatest elements
     */
    std::vector< std::vector< float > > 
    getFirstNMaxElemsPerNl
    ( 
        const std::vector< std::complex< float > > & data, 
        const size_t N, 
        const FftParams & params 
    );

    std::vector< StrobePoint >
    getFirstNMaxElemsPerNl_1
    ( 
        const std::vector< std::complex< float > > & data, 
        const size_t N, 
        const FftParams & params 
    );

    bool 
    isSimilarMatrix
    ( 
        const std::vector< std::vector< float > > & a, 
        const std::vector< std::vector< float > > & b,
        const float eps
    );

    bool 
    isSimilarMatrix
    ( 
        const std::vector< StrobePoint > & a, 
        const std::vector< StrobePoint > & b,
        const float eps
    );
}

#endif // HELPER_FUNCTIONS_HXX__