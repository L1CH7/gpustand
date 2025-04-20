#pragma once

#include <vector>
#include <complex>

#include <IHash.hxx>
#include <StrobePoint.hxx>

/**
 * 
 * class StrobeHash represent data into matrix M[i][j] of strobepoint:
 * M: NL x num_maximums
 * each M[i] = partial_sort_copy contatins first num_maximums maximums in that NL
 * M[i][j] = StrobePoint{ kgd, kgrs, amplitude }
 * 
 */
class StrobeHash : public IHash
{
public:
    using Generic = float; 
    using StrobeTp = std::vector< std::complex< Generic > >;
    using HashTp = std::vector< std::vector< StrobePoint > >; // do it in base class if possible
    using EpsTp = Generic;

    StrobeHash() = default;
    
    StrobeHash( const StrobeTp & data, const FftParams & params, const size_t num_maximums = 1, const EpsTp eps = EpsTp{} );

    StrobeHash( const StrobeHash & other )
    :   hash_( other.hash_ )
    {}

    StrobeHash( const HashTp & hash ) // do it in base class if possible
    :   hash_( hash )
    {}

    void to_json( json & j ) const override;
    
    void from_json( const json & j ) override;
private:
    HashTp hash_; // do it in base class if possible
    EpsTp eps_;

protected:
    bool equalTo( const IHash & other ) const override;
};


/**
 * JSON integration to print/read verification
 */
// void to_json( json & j, const StrobeHash & p )
// {
//     j = json( p.getHash() );
// }

// void from_json( const json & j, StrobeHash & p )
// {
//     StrobeHash::HashTp hash( j );
//     p = StrobeHash( hash );
// }
