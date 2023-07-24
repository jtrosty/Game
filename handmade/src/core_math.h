//#if !defined(ERROR_REMOVER)
//#include "core.h"
//#endif

#if !defined(CORE_MATH_H)

union v2 {
    struct {
        real32 x; 
        real32 y;
    };
    real32 E[2];
};

inline v2 V2(real32 x, real32 y) {
    v2 result;

    result.x = x;
    result.y = y;
    
    return result;
}

inline v2 operator*(real32 a, v2 b) {
    v2 result;

    result.x = a * b.x;
    result.y = a * b.y;

    return result;
}

// TODO: This is utilizing the above operator
inline v2 operator*(v2 b, real32 a) {
    v2 result = a * b;
    return result;
}

inline v2 operator*=(v2 &b, real32 a) {
    b = a * b;
    return b;
}

inline v2 operator-(v2 a) {
    v2 result;
    
    result.x = -a.x;
    result.y = -a.y;

    return result;
}

inline v2 operator+(v2 a, v2 b) {
    v2 result;

    result.x = a.x + b.x;
    result.y = a.y + b.y;

    return result;
}

inline v2 operator+=(v2 &a, v2 b) {

    a = a + b;

    return a;
}

inline v2 operator-(v2 a, v2 b) {
    v2 result;

    result.x = a.x - b.x;
    result.y = a.y - b.y;

    return result;
}

inline real32 math_square(real32 a) {

    real32 result = a * a;

    return result;
}

inline real32 math_inner(v2 a, v2 b) {

    real32 result = a.x * b.x + a.y * b.y;

    return result;
}

inline real32 math_lenghtSq(v2 a) {

    real32 result = math_inner(a, a);
    
    return result;
}


#define CORE_MATH_H
#endif
