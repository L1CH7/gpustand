#pragma once
#include <FftParams.hxx>
#include <JsonHelper.hxx>

/**
 * 
 * Class that representing coordinates of given matrix of data.
 * Converts given strobe_index into kgd and kgrs
 * strobe_index = nl*KGD*KGRS + kgd*KGRS + kgrs
 * 
 */
struct StrobePoint
{
    size_t kgd;
    size_t kgrs;
    float value;

    StrobePoint() = default;

    StrobePoint( size_t strobe_index, FftParams params, float value )
    :   kgrs( strobe_index % params.kgrs ),
        kgd( ( strobe_index / params.kgrs ) % params.kgd ),
        value( value )
    {
    }

    NLOHMANN_DEFINE_TYPE_INTRUSIVE( StrobePoint, kgd, kgrs, value );
};
