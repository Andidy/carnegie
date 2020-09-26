#ifndef ADY_MATH_H
#define ADY_MATH_H

#include "ady_types.h"
#include <math.h>

#define PI 3.14159265359f

inline f32 DegToRad(f32 angle)
{
    return (angle * (PI / 180.0f));
}

// Vector 2 ----------------------------------------------------------------

typedef struct vec2
{
  union
  {
    struct { f32 x; f32 y; };
    struct { f32 u; f32 v; };
    f32 data[2];
  };
} vec2;

inline vec2 Vec2(f32 x, f32 y)
{
  return { x, y };
}

typedef struct ivec2
{
  union
  {
    struct { i32 x; i32 y; };
    struct { i32 u; i32 v; };
    i32 data[2];
  };
} ivec2;

inline ivec2 IVec2(i32 x, i32 y)
{
  return { x, y };
}

// Vector 3 ----------------------------------------------------------------

typedef struct vec3
{
  union
  {
    struct { f32 x; f32 y; f32 z; };
    struct { f32 u; f32 v; f32 w; };
    struct { f32 r; f32 g; f32 b; };
    f32 data[3];
  };
} vec3;

inline vec3 Vec3(f32 x, f32 y, f32 z)
{
  return {x, y, z};
}

inline vec3 ZeroVec()
{
  return {0.0f, 0.0f, 0.0f};
}

inline vec3 OneVec()
{
  return {1.0f, 1.0f, 1.0f};
}

inline vec3 UpVec()
{
  return {0.0f, 1.0f, 0.0f};
}

inline vec3 NegVec(vec3 v1)
{
  return {-1.0f * v1.x, -1.0f * v1.y, -1.0f * v1.z};
}

inline vec3 AddVec(vec3 v1, vec3 v2)
{
  return {v1.x + v2.x, v1.y + v2.y, v1.z + v2.z};
}

inline vec3 SubVec(vec3 v1, vec3 v2)
{
  return {v1.x - v2.x, v1.y - v2.y, v1.z - v2.z};
}

inline vec3 ScaleVec(vec3 v1, f32 f)
{
  return {v1.x * f, v1.y * f, v1.z * f};
}

inline vec3 MulVec(vec3 v1, vec3 v2)
{
  return {v1.x * v2.x, v1.y * v2.y, v1.z * v2.z};
}

inline f32 Dot(vec3 v1, vec3 v2)
{
  return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

inline vec3 Cross(vec3 v1, vec3 v2)
{
  return {v1.y * v2.z - v1.z * v2.y,
          v1.z * v2.x - v1.x * v2.z,
          v1.x * v2.y - v1.y * v2.x};
}

inline f32 Distance(vec3 v1, vec3 v2)
{
  vec3 dist = SubVec(v2, v1);
  return Dot(dist, dist);
}

// TODO: Replace sqrtf with custom implementation?
inline f32 VecLen(vec3 v)
{
  return sqrtf(Dot(v, v));
}

inline vec3 NormVec(vec3 v)
{
  f32 len = VecLen(v);
  len = (len == 0.0f) ? 1.0f : 1.0f / len;
  return ScaleVec(v, len);
}

// Matrix ------------------------------------------------------------------
// Styled after raysan5's raymath, and HandmadeMath

typedef union mat4
{
  struct 
  {
    f32 m0, m4, m8, m12;
    f32 m1, m5, m9, m13;
    f32 m2, m6, m10, m14;
    f32 m3, m7, m11, m15;
  };
  
  f32 data[4][4];
  //f32 *ptr; //what is this even?
} mat4;

inline mat4 MulMat(mat4 m1, mat4 m2)
{
  mat4 result = {0};
  
  for(int col = 0; col < 4; ++col)
  {
    for(int row = 0; row < 4; ++row)
    {
      float sum = 0;
      for(int curr_mat = 0; curr_mat < 4; ++curr_mat)
      {
        sum += m1.data[curr_mat][row] * m2.data[col][curr_mat];
      }
      result.data[col][row] = sum;
    }
  }
  
  return (result);
}

inline mat4 DiagonalMat(f32 diag)
{
  mat4 result = {0};
  
  result.data[0][0] = diag;
  result.data[1][1] = diag;
  result.data[2][2] = diag;
  result.data[3][3] = diag;
  
  return (result);
}

inline mat4 TranslateMat(vec3 v)
{
  mat4 result = DiagonalMat(1.0f);
  
  result.data[3][0] = v.x;
  result.data[3][1] = v.y;
  result.data[3][2] = v.z;
  
  return (result);
}

// Note: angle is in degrees, axis will be normalized internally
inline mat4 RotateMat(f32 angle, vec3 axis)
{
  mat4 result = DiagonalMat(1.0f);
  axis = NormVec(axis);
  
  f32 SinTheta = sinf(DegToRad(angle));
  f32 CosTheta = cosf(DegToRad(angle));
  f32 CosValue = 1.0f - CosTheta;
  
  result.data[0][0] = (axis.x * axis.x * CosValue) + CosTheta;
  result.data[0][1] = (axis.x * axis.y * CosValue) + (axis.z * SinTheta);
  result.data[0][2] = (axis.x * axis.z * CosValue) - (axis.y * SinTheta);
  
  result.data[1][0] = (axis.y * axis.x * CosValue) - (axis.z * SinTheta);
  result.data[1][1] = (axis.y * axis.y * CosValue) + CosTheta;
  result.data[1][2] = (axis.y * axis.z * CosValue) + (axis.x * SinTheta);
  
  result.data[2][0] = (axis.z * axis.x * CosValue) + (axis.y * SinTheta);
  result.data[2][1] = (axis.z * axis.y * CosValue) - (axis.x * SinTheta);
  result.data[2][2] = (axis.z * axis.z * CosValue) + CosTheta;
  
  return (result);
}

inline mat4 ScaleMat(vec3 v)
{
  mat4 result = DiagonalMat(1.0f);
  
  result.data[0][0] = v.x;
  result.data[1][1] = v.y;
  result.data[2][2] = v.z;
  
  return (result);
}

// Note: fov asks for an angle in degrees
inline mat4 PerspectiveMat(f32 fov, f32 aspect_ratio, f32 near, f32 far)
{
  mat4 result = {0};
  
  f32 cotan = 1.0f / tanf(DegToRad(0.5f * fov));
  
  result.data[0][0] = cotan / aspect_ratio;
  result.data[1][1] = cotan;
  result.data[2][3] = 1.0f;
  result.data[2][2] = (far) / (far - near);
  result.data[3][2] = (-1.0f * near * far) / (far - near);
  result.data[3][3] = 0.0f;
  
  return (result);
}

inline mat4 OrthographicMat(f32 left, f32 right, f32 bot, f32 top, f32 near, f32 far)
{
  mat4 result = {0};
  
  result.data[0][0] = 2.0f / (right - left);
  result.data[1][1] = 2.0f / (top - bot);
  result.data[2][2] = 2.0f / (near - far);
  result.data[3][3] = 1.0f;
  
  result.data[3][0] = (left + right) / (left - right);
  result.data[3][1] = (bot + top) / (bot - top);
  result.data[3][2] = (far + near) / (near - far);
  
  return (result);
}

inline mat4 LookAtMat(vec3 eye, vec3 target, vec3 up)
{
  mat4 result;
  
  vec3 f = NormVec(SubVec(target, eye));
  vec3 s = NormVec(Cross(up, f));
  vec3 u = Cross(f, s);
  
  negeye = NegVec(eye);

  result.data[0][0]  = s.x;
  result.data[0][1]  = u.x;
  result.data[0][2]  = -f.x;
  result.data[0][3]  = 0.0f;
  
  result.data[1][0]  = s.y;
  result.data[1][1]  = u.y;
  result.data[1][2]  = -f.y;
  result.data[1][3]  = 0.0f;
  
  result.data[2][0]  = s.z;
  result.data[2][1]  = u.z;
  result.data[2][2] = -f.z;
  result.data[2][3] = 0.0f;
  
  result.data[3][0] = -Dot(s, eye);
  result.data[3][1] = -Dot(u, eye);
  result.data[3][2] = -Dot(f, eye);
  result.data[3][3] = 1.0f;
  
  return (result);
}

#endif