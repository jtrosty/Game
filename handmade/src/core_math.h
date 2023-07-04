#include <math.h>

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

inline i32 clampInt32(i32 value, i32 high_spec) {
    if (value > high_spec) {
        value = high_spec;
    } 
    return value;
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