#include "stdio.h"
#include "stdlib.h"
#include "math.h"

typedef unsigned int func_t;
typedef unsigned int string_t;
typedef float vec_t;

#include "mathlib/vector.h"
#include "mathlib/vector2d.h"

#undef DotProduct

inline float DotProduct(const Vector2D &a, const Vector2D &b)
{
	return (a.x * b.x + a.y * b.y);
}

inline float DotProduct(const Vector &a, const Vector &b)
{
	return(a.x*b.x+a.y*b.y+a.z*b.z);
}

inline Vector CrossProduct(const Vector &a, const Vector &b)
{
	return Vector(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

#ifndef DID_VEC3_T_DEFINE
#define DID_VEC3_T_DEFINE
#define vec3_t Vector
#endif