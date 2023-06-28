#include <math.h>

inline i32 roundReal32ToInt32(real32 r32) {
    i32 result = (i32)roundf(r32);
    return result;
}