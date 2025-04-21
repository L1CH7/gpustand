#include <StrobeHash.hxx>

#include <algorithm>

StrobeHash::StrobeHash( const StrobeTp & data, const FftParams & params, const size_t num_maximums, const Generic eps )
:   eps_( eps ),
    hash_( params.nl )
{
    /**
     * data array represents matrix NL*kgd*kgrs, we need to get N=3 max elements for each NL 
     */
    size_t nl = params.nl;
    size_t kgd = params.kgd;
    size_t kgrs = params.kgrs;
    size_t res_size = nl * kgd * kgrs;
    
    size_t max_find_num = num_maximums;
    if( max_find_num > data.size() )
        max_find_num = data.size();

    // Array on pairs index-value, where value is amplitude
    std::vector< std::pair< size_t, Generic > > data_indexed( data.size() );
    for( size_t i = 0; i < data.size(); ++i )
        data_indexed[i] = { i, std::abs( data[i] ) };

    // Array to store (max_find_num) amplitudes in corresponding NL
    decltype( data_indexed ) nl_maximums( max_find_num ); 

    size_t step = res_size / nl;
    auto data_nl_begin = data_indexed.begin();
    auto data_nl_end = data_indexed.begin() + step;
    for( size_t i = 0; i < nl; ++i )
    {
        std::partial_sort_copy
        ( 
            data_nl_begin, data_nl_end, 
            nl_maximums.begin(), nl_maximums.end(), 
            []( const std::pair< size_t, Generic > & a, const std::pair< size_t, Generic > & b )
            { 
                return a.second > b.second; 
            } 
        ); // get greatest (max_find_num) elems by amplitude
        
        for( auto & [idx, val] : nl_maximums )
            hash_.at( i ).push_back( StrobePoint{ idx, params, val } ); 

        data_nl_begin += step;
        data_nl_end += step;
    }
}

bool StrobeHash::equalTo( const IHash & other ) const 
{
    size_t size = hash_.size();
    const StrobeHash & that = dynamic_cast< const StrobeHash & >( other );
    auto other_hash = that.hash_;
    
    if( size != other_hash.size() )
        return false;

    if( size == 0 )
        return true;
    
    // check each nl
    for( size_t nl = 0; nl < size; ++nl )
    {
        bool is_similar = std::is_permutation
        ( 
            hash_.at( nl ).begin(), hash_.at( nl ).end(), 
            other_hash.at( nl ).begin(), other_hash.at( nl ).end(), 
            [this]( const StrobePoint & a, const StrobePoint & b )
            {
                return  std::abs( a.value - b.value ) * eps_ < 1.f
                        && a.kgd == b.kgd
                        && a.kgrs == b.kgrs;
            } 
        );
        if( !is_similar )
            return false;
    }
    return true;
}

void StrobeHash::to_json( json & j ) const 
{
    j = hash_;
}

void StrobeHash::from_json( const json & j )
{
    hash_ = HashTp( j );
}

