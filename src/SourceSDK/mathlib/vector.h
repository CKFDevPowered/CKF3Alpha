//====== Copyright ?1996-2005, Valve Corporation, All rights reserved. =======//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#ifndef VECTOR_H
#define VECTOR_H

#ifdef _WIN32
#pragma once
#endif

#include <math.h>
#include <float.h>

// For vec_t, put this somewhere else?
#include "tier0/basetypes.h"

// For rand(). We really need a library!
#include <stdlib.h>

#ifndef _X360
// For MMX intrinsics
#include <xmmintrin.h>
#endif

#include "tier0/dbg.h"
#include "tier0/threadtools.h"
#include "mathlib/vector2d.h"
#include "mathlib/math_pfns.h"
#include "minmax.h"

// Uncomment this to add extra Asserts to check for NANs, uninitialized vecs, etc.
//#define VECTOR_PARANOIA	1

// Uncomment this to make sure we don't do anything slow with our vectors
//#define VECTOR_NO_SLOW_OPERATIONS 1


// Used to make certain code easier to read.
#define X_INDEX	0
#define Y_INDEX	1
#define Z_INDEX	2


#ifdef VECTOR_PARANOIA
#define CHECK_VALID( _v)	Assert( (_v).IsValid() )
#else
#define CHECK_VALID( _v)	0
#endif

#define VecToString(v)	(static_cast<const char *>(CFmtStr("(%f, %f, %f)", (v).x, (v).y, (v).z))) // ** Note: this generates a temporary, don't hold reference!

class VectorByValue;

//=========================================================
// 3D Vector
//=========================================================
class Vector					
{
public:
	// Members
	vec_t x, y, z;

	// Construction/destruction:
	Vector(void); 
	Vector(vec_t X, vec_t Y, vec_t Z);
	Vector(const float *pFloat);
	
	// Initialization
	void Init(vec_t ix=0.0f, vec_t iy=0.0f, vec_t iz=0.0f);

	// Got any nasty NAN's?
	bool IsValid() const;
	void Invalidate();

	// array access...
	vec_t operator[](int i) const;
	vec_t& operator[](int i);

	// Base address...
	vec_t* Base();
	vec_t const* Base() const;

	// Cast to Vector2D...
	Vector2D& AsVector2D();
	const Vector2D& AsVector2D() const;

	// Initialization methods
	void Random( vec_t minVal, vec_t maxVal );
	inline void Zero(); ///< zero out a vector

	// equality
	bool operator==(const Vector& v) const;
	bool operator!=(const Vector& v) const;	

	// arithmetic operations
	FORCEINLINE Vector&	operator+=(const Vector &v);			
	FORCEINLINE Vector&	operator-=(const Vector &v);		
	FORCEINLINE Vector&	operator*=(const Vector &v);			
	FORCEINLINE Vector&	operator*=(float s);
	FORCEINLINE Vector&	operator/=(const Vector &v);		
	FORCEINLINE Vector&	operator/=(float s);	
	FORCEINLINE Vector&	operator+=(float fl) ; ///< broadcast add
	FORCEINLINE Vector&	operator-=(float fl) ; ///< broadcast sub			

// negate the vector components
	void	Negate(); 

	// Get the vector's magnitude.
	inline vec_t	Length() const;

	// Get the vector's magnitude squared.
	FORCEINLINE vec_t LengthSqr(void) const
	{ 
		CHECK_VALID(*this);
		return (x*x + y*y + z*z);		
	}

	// return true if this vector is (0,0,0) within tolerance
	bool IsZero( float tolerance = 0.01f ) const
	{
		return (x > -tolerance && x < tolerance &&
				y > -tolerance && y < tolerance &&
				z > -tolerance && z < tolerance);
	}

	vec_t	NormalizeInPlace();
	bool	IsLengthGreaterThan( float val ) const;
	bool	IsLengthLessThan( float val ) const;

	// check if a vector is within the box defined by two other vectors
	FORCEINLINE bool WithinAABox( Vector const &boxmin, Vector const &boxmax);
 
	// Get the distance from this vector to the other one.
	vec_t	DistTo(const Vector &vOther) const;

	// Get the distance from this vector to the other one squared.
	// NJS: note, VC wasn't inlining it correctly in several deeply nested inlines due to being an 'out of line' inline.  
	// may be able to tidy this up after switching to VC7
	FORCEINLINE vec_t DistToSqr(const Vector &vOther) const
	{
		Vector delta;

		delta.x = x - vOther.x;
		delta.y = y - vOther.y;
		delta.z = z - vOther.z;

		return delta.LengthSqr();
	}

	// Copy
	void	CopyToArray(float* rgfl) const;	

	// Multiply, add, and assign to this (ie: *this = a + b * scalar). This
	// is about 12% faster than the actual vector equation (because it's done per-component
	// rather than per-vector).
	void	MulAdd(const Vector& a, const Vector& b, float scalar);	

	// Dot product.
	vec_t	Dot(const Vector& vOther) const;			

	// assignment
	Vector& operator=(const Vector &vOther);

	// compatability for HL1
	Vector		Normalize(void) const;
	Vector2D	Make2D(void) const;

	// 2d
	vec_t	Length2D(void) const;					
	vec_t	Length2DSqr(void) const;					

	operator float *()						{ return &x; } // Vectors will now automatically convert to float * when needed
	operator const float *() const			{ return &x; } // Vectors will now automatically convert to float * when needed
	operator VectorByValue &()				{ return *((VectorByValue *)(this)); }
	operator const VectorByValue &() const	{ return *((const VectorByValue *)(this)); }

#ifndef VECTOR_NO_SLOW_OPERATIONS
	// copy constructors
//	Vector(const Vector &vOther);

	// arithmetic operations
	Vector	operator-(void) const;
				
	Vector	operator+(const Vector& v) const;	
	Vector	operator-(const Vector& v) const;	
	Vector	operator*(const Vector& v) const;	
	Vector	operator/(const Vector& v) const;	
	Vector	operator*(float fl) const;
	Vector	operator/(float fl) const;			
	
	// Cross product between two vectors.
	Vector	Cross(const Vector &vOther) const;		

	// Returns a vector with the min or max in X, Y, and Z.
	Vector	Min(const Vector &vOther) const;
	Vector	Max(const Vector &vOther) const;

#else

private:
	// No copy constructors allowed if we're in optimal mode
	Vector(const Vector& vOther);
#endif
};



#define USE_M64S ( ( !defined( _X360 ) ) && ( ! defined( _LINUX) ) )



//-----------------------------------------------------------------------------
// Allows us to specifically pass the vector by value when we need to
//-----------------------------------------------------------------------------
class VectorByValue : public Vector
{
public:
	// Construction/destruction:
	VectorByValue(void) : Vector() {} 
	VectorByValue(vec_t X, vec_t Y, vec_t Z) : Vector( X, Y, Z ) {}
	VectorByValue(const VectorByValue& vOther) { *this = vOther; }
};

//-----------------------------------------------------------------------------
//
// Inlined Vector methods
//
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// constructors
//-----------------------------------------------------------------------------
inline Vector::Vector(void)									
{ 
#ifdef _DEBUG
#ifdef VECTOR_PARANOIA
	// Initialize to NAN to catch errors
	x = y = z = VEC_T_NAN;
#endif
#endif
}

inline Vector::Vector(vec_t X, vec_t Y, vec_t Z)						
{ 
	x = X; y = Y; z = Z;
	CHECK_VALID(*this);
}

inline Vector::Vector(const float *pFloat)					
{
	Assert( pFloat );
	x = pFloat[0]; y = pFloat[1]; z = pFloat[2];	
	CHECK_VALID(*this);
} 

#if 0
//-----------------------------------------------------------------------------
// copy constructor
//-----------------------------------------------------------------------------

inline Vector::Vector(const Vector &vOther)					
{ 
	CHECK_VALID(vOther);
	x = vOther.x; y = vOther.y; z = vOther.z;
}
#endif

//-----------------------------------------------------------------------------
// initialization
//-----------------------------------------------------------------------------

inline void Vector::Init( vec_t ix, vec_t iy, vec_t iz )    
{ 
	x = ix; y = iy; z = iz;
	CHECK_VALID(*this);
}

inline void Vector::Random( vec_t minVal, vec_t maxVal )
{
	x = minVal + ((float)rand() / RAND_MAX) * (maxVal - minVal);
	y = minVal + ((float)rand() / RAND_MAX) * (maxVal - minVal);
	z = minVal + ((float)rand() / RAND_MAX) * (maxVal - minVal);
	CHECK_VALID(*this);
}

// This should really be a single opcode on the PowerPC (move r0 onto the vec reg)
inline void Vector::Zero()
{
	x = y = z = 0.0f;
}

//-----------------------------------------------------------------------------
// assignment
//-----------------------------------------------------------------------------

inline Vector& Vector::operator=(const Vector &vOther)	
{
	CHECK_VALID(vOther);
	x=vOther.x; y=vOther.y; z=vOther.z; 
	return *this; 
}


//-----------------------------------------------------------------------------
// Array access
//-----------------------------------------------------------------------------
inline vec_t& Vector::operator[](int i)
{
	Assert( (i >= 0) && (i < 3) );
	return ((vec_t*)this)[i];
}

inline vec_t Vector::operator[](int i) const
{
	Assert( (i >= 0) && (i < 3) );
	return ((vec_t*)this)[i];
}


//-----------------------------------------------------------------------------
// Base address...
//-----------------------------------------------------------------------------
inline vec_t* Vector::Base()
{
	return (vec_t*)this;
}

inline vec_t const* Vector::Base() const
{
	return (vec_t const*)this;
}

//-----------------------------------------------------------------------------
// Cast to Vector2D...
//-----------------------------------------------------------------------------

inline Vector2D& Vector::AsVector2D()
{
	return *(Vector2D*)this;
}

inline const Vector2D& Vector::AsVector2D() const
{
	return *(const Vector2D*)this;
}

//-----------------------------------------------------------------------------
// IsValid?
//-----------------------------------------------------------------------------

inline bool Vector::IsValid() const
{
	return IsFinite(x) && IsFinite(y) && IsFinite(z);
}

//-----------------------------------------------------------------------------
// Invalidate
//-----------------------------------------------------------------------------

inline void Vector::Invalidate()
{
//#ifdef _DEBUG
//#ifdef VECTOR_PARANOIA
	x = y = z = VEC_T_NAN;
//#endif
//#endif
}

//-----------------------------------------------------------------------------
// comparison
//-----------------------------------------------------------------------------

inline bool Vector::operator==( const Vector& src ) const
{
	CHECK_VALID(src);
	CHECK_VALID(*this);
	return (src.x == x) && (src.y == y) && (src.z == z);
}

inline bool Vector::operator!=( const Vector& src ) const
{
	CHECK_VALID(src);
	CHECK_VALID(*this);
	return (src.x != x) || (src.y != y) || (src.z != z);
}


//-----------------------------------------------------------------------------
// Copy
//-----------------------------------------------------------------------------

inline void	Vector::CopyToArray(float* rgfl) const		
{ 
	Assert( rgfl );
	CHECK_VALID(*this);
	rgfl[0] = x, rgfl[1] = y, rgfl[2] = z; 
}

//-----------------------------------------------------------------------------
// standard math operations
//-----------------------------------------------------------------------------
// #pragma message("TODO: these should be SSE")

inline void Vector::Negate()
{ 
	CHECK_VALID(*this);
	x = -x; y = -y; z = -z; 
} 

FORCEINLINE  Vector& Vector::operator+=(const Vector& v)	
{ 
	CHECK_VALID(*this);
	CHECK_VALID(v);
	x+=v.x; y+=v.y; z += v.z;	
	return *this;
}

FORCEINLINE  Vector& Vector::operator-=(const Vector& v)	
{ 
	CHECK_VALID(*this);
	CHECK_VALID(v);
	x-=v.x; y-=v.y; z -= v.z;	
	return *this;
}

FORCEINLINE  Vector& Vector::operator*=(float fl)	
{
	x *= fl;
	y *= fl;
	z *= fl;
	CHECK_VALID(*this);
	return *this;
}

FORCEINLINE  Vector& Vector::operator*=(const Vector& v)	
{ 
	CHECK_VALID(v);
	x *= v.x;
	y *= v.y;
	z *= v.z;
	CHECK_VALID(*this);
	return *this;
}

// this ought to be an opcode.
FORCEINLINE Vector&	Vector::operator+=(float fl) 
{
	x += fl;
	y += fl;
	z += fl;
	CHECK_VALID(*this);
	return *this;
}

FORCEINLINE Vector&	Vector::operator-=(float fl) 
{
	x -= fl;
	y -= fl;
	z -= fl;
	CHECK_VALID(*this);
	return *this;
}



FORCEINLINE  Vector& Vector::operator/=(float fl)	
{
	Assert( fl != 0.0f );
	float oofl = 1.0f / fl;
	x *= oofl;
	y *= oofl;
	z *= oofl;
	CHECK_VALID(*this);
	return *this;
}

FORCEINLINE  Vector& Vector::operator/=(const Vector& v)	
{ 
	CHECK_VALID(v);
	Assert( v.x != 0.0f && v.y != 0.0f && v.z != 0.0f );
	x /= v.x;
	y /= v.y;
	z /= v.z;
	CHECK_VALID(*this);
	return *this;
}

FORCEINLINE void VectorAdd( const Vector& a, const Vector& b, Vector& c )
{
	CHECK_VALID(a);
	CHECK_VALID(b);
	c.x = a.x + b.x;
	c.y = a.y + b.y;
	c.z = a.z + b.z;
}

FORCEINLINE void VectorSubtract( const Vector& a, const Vector& b, Vector& c )
{
	CHECK_VALID(a);
	CHECK_VALID(b);
	c.x = a.x - b.x;
	c.y = a.y - b.y;
	c.z = a.z - b.z;
}

FORCEINLINE void VectorMultiply( const Vector& a, vec_t b, Vector& c )
{
	CHECK_VALID(a);
	Assert( IsFinite(b) );
	c.x = a.x * b;
	c.y = a.y * b;
	c.z = a.z * b;
}

FORCEINLINE void VectorMultiply( const Vector& a, const Vector& b, Vector& c )
{
	CHECK_VALID(a);
	CHECK_VALID(b);
	c.x = a.x * b.x;
	c.y = a.y * b.y;
	c.z = a.z * b.z;
}

// for backwards compatability
inline void VectorScale ( const Vector& in, vec_t scale, Vector& result )
{
	VectorMultiply( in, scale, result );
}


FORCEINLINE void VectorDivide( const Vector& a, vec_t b, Vector& c )
{
	CHECK_VALID(a);
	Assert( b != 0.0f );
	vec_t oob = 1.0f / b;
	c.x = a.x * oob;
	c.y = a.y * oob;
	c.z = a.z * oob;
}

FORCEINLINE void VectorDivide( const Vector& a, const Vector& b, Vector& c )
{
	CHECK_VALID(a);
	CHECK_VALID(b);
	Assert( (b.x != 0.0f) && (b.y != 0.0f) && (b.z != 0.0f) );
	c.x = a.x / b.x;
	c.y = a.y / b.y;
	c.z = a.z / b.z;
}

// FIXME: Remove
// For backwards compatability
inline void	Vector::MulAdd(const Vector& a, const Vector& b, float scalar)
{
	CHECK_VALID(a);
	CHECK_VALID(b);
	x = a.x + b.x * scalar;
	y = a.y + b.y * scalar;
	z = a.z + b.z * scalar;
}

inline void VectorLerp(const Vector& src1, const Vector& src2, vec_t t, Vector& dest )
{
	CHECK_VALID(src1);
	CHECK_VALID(src2);
	dest.x = src1.x + (src2.x - src1.x) * t;
	dest.y = src1.y + (src2.y - src1.y) * t;
	dest.z = src1.z + (src2.z - src1.z) * t;
}


//-----------------------------------------------------------------------------
// length
//-----------------------------------------------------------------------------

inline vec_t Vector::Length(void) const	
{
	CHECK_VALID(*this);
	return (vec_t)sqrt(x*x + y*y + z*z);	
}

inline vec_t Vector::NormalizeInPlace()
{
	float radius = Length();

	if (radius)
	{
		// FLT_EPSILON is added to the radius to eliminate the possibility of divide by zero.
		float iradius = 1.f / ( radius + FLT_EPSILON );
	
		x *= iradius;
		y *= iradius;
		z *= iradius;
	}
	else
	{
		// FIXME: 
		// Just copying the existing implemenation; shouldn't res.z == 0?
		x = y = 0.0f; z = 1.0f;
	}

	return radius;
}

inline bool Vector::IsLengthGreaterThan( float val ) const
{
	return LengthSqr() > val*val;
}

inline bool Vector::IsLengthLessThan( float val ) const
{
	return LengthSqr() < val*val;
}


//-----------------------------------------------------------------------------
//
// Slow methods
//
//-----------------------------------------------------------------------------

#ifndef VECTOR_NO_SLOW_OPERATIONS

//-----------------------------------------------------------------------------
// Returns a vector with the min or max in X, Y, and Z.
//-----------------------------------------------------------------------------
inline Vector Vector::Min(const Vector &vOther) const
{
	return Vector(x < vOther.x ? x : vOther.x, 
		y < vOther.y ? y : vOther.y, 
		z < vOther.z ? z : vOther.z);
}

inline Vector Vector::Max(const Vector &vOther) const
{
	return Vector(x > vOther.x ? x : vOther.x, 
		y > vOther.y ? y : vOther.y, 
		z > vOther.z ? z : vOther.z);
}


//-----------------------------------------------------------------------------
// arithmetic operations
//-----------------------------------------------------------------------------

inline Vector Vector::operator-(void) const
{ 
	return Vector(-x,-y,-z);				
}

inline Vector Vector::operator+(const Vector& v) const	
{ 
	CHECK_VALID(v);
	return Vector(x + v.x, y + v.y, z + v.z);	
}

inline Vector Vector::operator-(const Vector& v) const	
{ 
	CHECK_VALID(v);
	return Vector(x - v.x, y - v.y, z - v.z);	
}

inline Vector Vector::operator*(float fl) const	
{ 
	return Vector(x * fl, y * fl, z * fl);	
}

inline Vector Vector::operator*(const Vector& v) const	
{ 
	CHECK_VALID(v);
	return Vector(x * v.x, y * v.y, z * v.z);	
}

inline Vector Vector::operator/(float fl) const	
{ 
	Assert( fl != 0.0f );
	vec_t oob = 1.0f / fl;
	return Vector(x * oob, y * oob, z * oob);
}

inline Vector Vector::operator/(const Vector& v) const	
{ 

	CHECK_VALID(v);
	Assert( (v.x != 0.0f) && (v.y != 0.0f) && (v.z != 0.0f) );
	return Vector(x / v.x, y / v.y, z / v.z);	
}

inline Vector operator*(float fl, const Vector& v)	
{ 
	return v * fl; 
}

//-----------------------------------------------------------------------------
// cross product
//-----------------------------------------------------------------------------

inline Vector Vector::Cross(const Vector& vOther) const
{ 
	CHECK_VALID(vOther);
	return Vector(y*vOther.z - z*vOther.y, z*vOther.x - x*vOther.z, x*vOther.y - y*vOther.x);
}

//-----------------------------------------------------------------------------
// compatability for HL1
//-----------------------------------------------------------------------------

inline Vector Vector::Normalize(void) const
{
	float flLen = Length();
	if (flLen == 0) return Vector(0,0,1); // ????
	flLen = 1 / flLen;
	return Vector(x * flLen, y * flLen, z * flLen);
}

inline Vector2D Vector::Make2D(void) const
{
	Vector2D	Vec2;

	Vec2.x = x;
	Vec2.y = y;

	return Vec2;
}

//-----------------------------------------------------------------------------
// 2D
//-----------------------------------------------------------------------------

inline vec_t Vector::Length2D(void) const
{ 
	return (vec_t)sqrt(x*x + y*y); 
}

inline vec_t Vector::Length2DSqr(void) const
{ 
	return (x*x + y*y); 
}

#endif //slow

//-----------------------------------------------------------------------------
// Helper debugging stuff....
//-----------------------------------------------------------------------------

inline bool operator==( float const* f, const Vector& v )
{
	// AIIIEEEE!!!!
	Assert(0);
	return false;
}

inline bool operator==( const Vector& v, float const* f )
{
	// AIIIEEEE!!!!
	Assert(0);
	return false;
}

inline bool operator!=( float const* f, const Vector& v )
{
	// AIIIEEEE!!!!
	Assert(0);
	return false;
}

inline bool operator!=( const Vector& v, float const* f )
{
	// AIIIEEEE!!!!
	Assert(0);
	return false;
}

#endif

