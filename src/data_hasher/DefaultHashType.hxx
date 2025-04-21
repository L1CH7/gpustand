#pragma once

/**
 * Include any IHash realization here:
 * That header is using inside DataHashWrapper for customising hash type and logic
 */
#include <StrobeHash.hxx>
using DefaultHashType = StrobeHash;
