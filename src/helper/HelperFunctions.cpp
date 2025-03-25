#include <HelperFunctions.h>

/**
 * splits array into NL parts
 * from every part of it gets N greatest elements
 */
std::vector< std::vector< float > > 
Helper::getFirstNMaxElemsPerNl
( 
    const std::vector< std::complex< float > > & data, 
    const size_t N, 
    const FftParams & params 
)
{
    /* data array represents matrix NL*kgd*kgrs, we need to get N=3 max elements for each NL */
    size_t res_size = params.nl * params.kgd * params.kgrs;

    size_t max_find_num = N;
    if( max_find_num > data.size() )
        max_find_num = data.size();

    // std::vector< std::complex< float > > res( max_find_num );
    std::remove_cvref_t< decltype( data ) > res( max_find_num ); // same type T as data, but removed const T&
    std::vector< std::vector< float > > res_nl( params.nl, std::vector< float >( res.size() ) );

    size_t step = res_size / params.nl;
    auto data_nl_begin = data.begin();
    auto data_nl_end = data.begin() + step;
    for( size_t i = 0; i < params.nl; ++i )
    {
        std::partial_sort_copy
        ( 
            data_nl_begin, data_nl_end, 
            res.begin(), res.end(), 
            []( std::complex< float > a, std::complex< float > b )
            { 
                return std::abs( a ) > std::abs( b ); 
            } 
        ); // get greatest N elems by amplitude
        // res_nl.emplace_back( res );
        std::transform
        ( 
            res.begin(), res.end(), res_nl.at( i ).begin(), 
            []( std::complex< float > x ) -> float 
            {
                return std::abs( x );
            }
        ); // makes vector of amplitudes
        data_nl_begin += step;
        data_nl_end += step;
    }
    return res_nl;
}

bool 
Helper::isSimilarMatrix
( 
    const std::vector< std::vector< float > > & a, 
    const std::vector< std::vector< float > > & b,
    const float eps
)
{
    size_t size = a.size();
    if( size != b.size() )
        return false;

    if( size == 0 )
        return true;

    for( size_t i = 0; i < size; ++i )
    {
        if( a.at( i ).size() != b.at( i ).size() )
            return false;

        bool is_similar = std::is_permutation( 
            a.at( i ).begin(), a.at( i ).end(), 
            b.at( i ).begin(), b.at( i ).end(), 
            [eps]( const float a, const float b ){
                return std::abs( a - b ) * eps < 1.f;
            } 
        );
        if( !is_similar )
            return false;
    }   
    return true;
}