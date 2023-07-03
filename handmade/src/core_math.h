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