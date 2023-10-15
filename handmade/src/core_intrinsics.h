
//#if !defined(ERROR_REMOVER)
//#include "core.h"
//#endif
#if !defined(CORE_INTRINSICS_H)

#include <math.h>

inline real32 absoluteValue(real32 r32) {
    real32 result = fabs(r32);
    return result;
}

inline i32 signOf(i32 value) {
    i32 result = (value >= 0) ? 1 : -1;
    return result;
}

inline i32 roundReal32ToInt32(real32 r32) {
    i32 result = (i32)roundf(r32);
    return result;
}

inline u32 roundReal32ToUint32(real32 r32) {
    u32 result = (u32)roundf(r32);
    return result;
}

inline i32 truncateReal32ToInt32(real32 r32) {
    i32 result = (i32)r32;
    return result;
}

inline i32 floorReal32ToInt32(real32 r32) {
    i32 result = (i32)floorf(r32);
    return result;
}

inline i32 ceilReal32ToInt32(real32 r32) {
    i32 result = (i32)ceilf(r32);
    return result;
}

inline i32 clampInt32(i32 value, i32 high_spec) {
    if (value > high_spec) {
        value = high_spec;
    } 
    return value;
}

inline real32 squareRoot(real32 r32) {
    real32 result = sqrtf(r32);
    return result;
}

inline i32 clampAbsValueInt32(i32 value, i32 high_spec) {
    if (value > high_spec) {
        value = high_spec;
    } 
    else if (value < -high_spec) {
        value = -high_spec;
    }
    return value;
}

inline real32 clampAbsValueReal32(real32 value, real32 high_spec) {
    if (value > high_spec) {
        value = high_spec;
    } 
    else if (value < -high_spec) {
        value = -high_spec;
    }
    return value;
}
// Math files

inline real32 core_sin(real32 angle) {
    real32 result = sinf(angle);
    return result;
}

inline real32 core_cos(real32 angle) {
    real32 result = sinf(angle);
    return result;
}

inline real32 core_atan2(real32 y, real32 x) {
    real32 result = atan2f(y, x);
    return result;
}

struct Bit_Scan_Result {
    bool32 found;
    u32 index;
};
inline Bit_Scan_Result findLeastSignificantSetBit(u32 value) {
    Bit_Scan_Result result = {};

#if COMPILER_MSVC
        result.found = _BitScanForward((unsigned long*)&result.index, value);
#else
        for (u32 test = 0; test < 32; ++test) {
            if (value & (1 << test)) {
                result.index = test;
                result.found = true;
                break;
            }
        }
#endif
        return result;
}

static u32 rotateLeft(u32 Value, i32 Amount)
{
#if COMPILER_MSVC
    uint32 result = _rotl(Value, Amount);
#else
    // TODO(casey): Actually port this to other compiler platforms!
    Amount &= 31;
    uint32 result = ((Value << Amount) | (Value >> (32 - Amount)));
#endif

    return(result);
}

static u32 rotateRight(u32 Value, i32 Amount) {
#if COMPILER_MSVC
    uint32 Result = _rotr(Value, Amount);
#else
    // TODO(casey): Actually port this to other compiler platforms!
    Amount &= 31;
    uint32 Result = ((Value >> Amount) | (Value << (32 - Amount)));
#endif

    return(Result);
}


#define CORE_INTRINSICS_H
#endif
