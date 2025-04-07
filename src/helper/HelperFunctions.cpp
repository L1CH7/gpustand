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

        bool is_similar = std::is_permutation
        ( 
            a.at( i ).begin(), a.at( i ).end(), 
            b.at( i ).begin(), b.at( i ).end(), 
            [eps]( const float a, const float b )
            {
                return std::abs( a - b ) * eps < 1.f;
            } 
        );
        if( !is_similar )
            return false;
    }   
    return true;
}

// #include <map>
// #include <tuple>
// std::vector< std::map< std::pair< size_t, size_t >, float > >
// std::map< std::tuple< size_t, size_t, size_t >, float >
// std::vector< std::pair< size_t, float > >
std::vector< StrobeResPoint >
Helper::getFirstNMaxElemsPerNl_1
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

    // Array on pairs index-value, where value 
    std::vector< std::pair< size_t, float > > data_indexed( data.size() );
    for( size_t i = 0; i < data.size(); ++i )
        data_indexed[i] = { i, std::abs( data[i] ) };

    // std::remove_cvref_t< decltype( data ) > res( max_find_num ); // same type T as data, but removed const T&
    decltype( data_indexed ) res( max_find_num );
    std::vector< StrobeResPoint > res_nl;

    size_t step = res_size / params.nl;
    auto data_nl_begin = data_indexed.begin();
    auto data_nl_end = data_indexed.begin() + step;
    for( size_t i = 0; i < params.nl; ++i )
    {
        std::partial_sort_copy
        ( 
            data_nl_begin, data_nl_end, 
            res.begin(), res.end(), 
            []( const std::pair< size_t, float > & a, const std::pair< size_t, float > & b )
            { 
                return a.second > b.second; 
            } 
        ); // get greatest N elems by amplitude
        
        for( auto & [idx, val] : res )
            res_nl.push_back( StrobeResPoint{ idx, val, params } ); 

        data_nl_begin += step;
        data_nl_end += step;
    }
    return res_nl;
}

bool 
Helper::isSimilarMatrix
( 
    const std::vector< StrobeResPoint > & a, 
    const std::vector< StrobeResPoint > & b,
    const float eps
)
{
    size_t size = a.size();
    if( size != b.size() )
        return false;

    if( size == 0 )
        return true;

    bool is_similar = std::is_permutation
    ( 
        a.begin(), a.end(), 
        b.begin(), b.end(), 
        [eps]( const StrobeResPoint & a, const StrobeResPoint & b )
        {
            return  std::abs( a.value - b.value ) * eps < 1.f &&
                    a.nl == b.nl &&
                    a.kgd == b.kgd &&
                    a.kgrs == b.kgrs;
        } 
    );
    return is_similar;
}