// #if !defined(ERROR_REMOVER)
// #include "core.h"
// #endif

#if !defined(CORE_MATH_H)

union v2 {
  struct {
    real32 x;
    real32 y;
  };
  real32 E[2];
};

union v3 {
  struct {
    real32 x, y, z;
  };
  struct {
    real32 r, g, b;
  };
  real32 e[3];
};

union v4 {
  struct {
    real32 x, y, z, w;
  };
  struct {
    real32 r, g, b, a;
  };
  real32 e[4];
};

inline v2 V2(real32 x, real32 y) {
  v2 result;

  result.x = x;
  result.y = y;

  return result;
}

inline v3 V3(real32 x, real32 y, real32 z) {
  v3 result;

  result.x = x;
  result.y = y;
  result.z = z;

  return result;
}

inline v4 V4(real32 x, real32 y, real32 z, real32 w) {
  v4 result;

  result.x = x;
  result.y = y;
  result.z = z;
  result.w = w;

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

inline v2 operator*=(v2& b, real32 a) {
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

inline v2 operator+=(v2& a, v2 b) {

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

struct Rectangle2 {
  v2 min;
  v2 max;
};

inline v2 getMinCorner(Rectangle2 rect) {
  v2 result = rect.min;
  return result;
}

inline v2 getMaxCorner(Rectangle2 rect) {
  v2 result = rect.max;
  return result;
}

inline v2 getCenter(Rectangle2 rect) {
  v2 result = 0.5f * (rect.min + rect.max);
  return result;
}

inline Rectangle2 rectMinMax(v2 min, v2 max) {
  Rectangle2 result;
  result.min = min;
  result.max = max;

  return result;
}

inline Rectangle2 rectMinDim(v2 min, v2 dim) {
  Rectangle2 result;
  result.min = min;
  result.max = min + dim;

  return result;
}

inline Rectangle2 rectCenterHalfDim(v2 center, v2 half_dim) {
  Rectangle2 result;
  result.min = center - half_dim;
  result.max = center + half_dim;

  return result;
}

inline Rectangle2 rectCenterDim(v2 center, v2 dim) {
  Rectangle2 result = rectCenterHalfDim(center, 0.5 * dim);

  return result;
}

inline bool32 isInRectangle(Rectangle2 rectangle, v2 test) {
  bool32 result = ((test.x >= rectangle.min.x) && (test.y >= rectangle.min.y) &&
                   (test.x < rectangle.max.x) && (test.y < rectangle.max.y));
  return result;
}

#define CORE_MATH_H
#endif
