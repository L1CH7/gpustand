#ifndef HELPER_FUNCTIONS_H__
#define HELPER_FUNCTIONS_H__

// #include <vector>
// #include <complex>
#include <types.h>
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

    bool 
    isSimilarMatrix
    ( 
        const std::vector< std::vector< float > > & a, 
        const std::vector< std::vector< float > > & b,
        const float eps
    );
}

#endif // HELPER_FUNCTIONS_H__