//===== Copyright ?1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
//===========================================================================//

#ifndef MATH_LIB_H
#define MATH_LIB_H

#include <math.h>
#include "tier0/basetypes.h"
#include "mathlib/vector.h"
#include "mathlib/vector2d.h"
#include "tier0/dbg.h"

#ifndef M_PI
	#define M_PI		3.14159265358979323846	// matches value in gcc v2 math.h
#endif

#define M_PI_F		((float)(M_PI))	// Shouldn't collide with anything.

// NJS: Inlined to prevent floats from being autopromoted to doubles, as with the old system.
#ifndef RAD2DEG
	#define RAD2DEG( x  )  ( (float)(x) * (float)(180.f / M_PI_F) )
#endif

#ifndef DEG2RAD
	#define DEG2RAD( x  )  ( (float)(x) * (float)(M_PI_F / 180.f) )
#endif

// Math routines done in optimized assembly math package routines
void inline SinCos( float radians, float *sine, float *cosine )
{
#if defined( _X360 )
	XMScalarSinCos( sine, cosine, radians );
#elif defined( _WIN32 )
	_asm
	{
		fld		DWORD PTR [radians]
		fsincos

		mov edx, DWORD PTR [cosine]
		mov eax, DWORD PTR [sine]

		fstp DWORD PTR [edx]
		fstp DWORD PTR [eax]
	}
#elif defined( _LINUX )
	register double __cosr, __sinr;
 	__asm __volatile__
    		("fsincos"
     	: "=t" (__cosr), "=u" (__sinr) : "0" (radians));

  	*sine = __sinr;
  	*cosine = __cosr;
#endif
}

#define SIN_TABLE_SIZE	256
#define FTOIBIAS		12582912.f
extern float SinCosTable[SIN_TABLE_SIZE];

inline float TableCos( float theta )
{
	union
	{
		int i;
		float f;
	} ftmp;

	// ideally, the following should compile down to: theta * constant + constant, changing any of these constants from defines sometimes fubars this.
	ftmp.f = theta * ( float )( SIN_TABLE_SIZE / ( 2.0f * M_PI ) ) + ( FTOIBIAS + ( SIN_TABLE_SIZE / 4 ) );
	return SinCosTable[ ftmp.i & ( SIN_TABLE_SIZE - 1 ) ];
}

inline float TableSin( float theta )
{
	union
	{
		int i;
		float f;
	} ftmp;

	// ideally, the following should compile down to: theta * constant + constant
	ftmp.f = theta * ( float )( SIN_TABLE_SIZE / ( 2.0f * M_PI ) ) + FTOIBIAS;
	return SinCosTable[ ftmp.i & ( SIN_TABLE_SIZE - 1 ) ];
}

template<class T>
FORCEINLINE T Square( T const &a )
{
	return a * a;
}


FORCEINLINE bool IsPowerOfTwo( uint x )
{
	return ( x & ( x - 1 ) ) == 0;
}

// return the smallest power of two >= x.
// returns 0 if x == 0 or x > 0x80000000 (ie numbers that would be negative if x was signed)
// NOTE: the old code took an int, and if you pass in an int of 0x80000000 casted to a uint,
//       you'll get 0x80000000, which is correct for uints, instead of 0, which was correct for ints
FORCEINLINE uint SmallestPowerOfTwoGreaterOrEqual( uint x )
{
	x -= 1;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	return x + 1;
}

// return the largest power of two <= x. Will return 0 if passed 0
FORCEINLINE uint LargestPowerOfTwoLessThanOrEqual( uint x )
{
	if ( x >= 0x80000000 )
		return 0x80000000;

	return SmallestPowerOfTwoGreaterOrEqual( x + 1 ) >> 1;
}


// Math routines for optimizing division
void FloorDivMod (double numer, double denom, int *quotient, int *rem);
int GreatestCommonDivisor (int i1, int i2);

// Test for FPU denormal mode
bool IsDenormal( const float &val );

// Remap a value in the range [A,B] to [C,D].
inline float RemapVal( float val, float A, float B, float C, float D)
{
	if ( A == B )
		return val >= B ? D : C;
	return C + (D - C) * (val - A) / (B - A);
}

inline float RemapValClamped( float val, float A, float B, float C, float D)
{
	if ( A == B )
		return val >= B ? D : C;
	float cVal = (val - A) / (B - A);
	cVal = clamp( cVal, 0.0f, 1.0f );

	return C + (D - C) * cVal;
}

// Returns A + (B-A)*flPercent.
// float Lerp( float flPercent, float A, float B );
template <class T>
FORCEINLINE T Lerp( float flPercent, T const &A, T const &B )
{
	return A + (B - A) * flPercent;
}

// 5-argument floating point linear interpolation.
// FLerp(f1,f2,i1,i2,x)=
//    f1 at x=i1
//    f2 at x=i2
//   smooth lerp between f1 and f2 at x>i1 and x<i2
//   extrapolation for x<i1 or x>i2
//
//   If you know a function f(x)'s value (f1) at position i1, and its value (f2) at position i2,
//   the function can be linearly interpolated with FLerp(f1,f2,i1,i2,x)
//    i2=i1 will cause a divide by zero.
static inline float FLerp(float f1, float f2, float i1, float i2, float x)
{
  return f1+(f2-f1)*(x-i1)/(i2-i1);
}

// Swap two of anything.
template <class T> 
FORCEINLINE void swap( T& x, T& y )
{
	T temp = x;
	x = y;
	y = temp;
}

template <class T> FORCEINLINE T AVG(T a, T b)
{
	return (a+b)/2;
}

// number of elements in an array of static size
#define NELEMS(x) ((sizeof(x))/sizeof(x[0]))

// XYZ macro, for printf type functions - ex printf("%f %f %f",XYZ(myvector));
#define XYZ(v) (v).x,(v).y,(v).z

//
// Returns a clamped value in the range [min, max].
//
#define clamp(val, min, max) (((val) > (max)) ? (max) : (((val) < (min)) ? (min) : (val)))

inline float Sign( float x )
{
	return (x <0.0f) ? -1.0f : 1.0f;
}

//
// Clamps the input integer to the given array bounds.
// Equivalent to the following, but without using any branches:
//
// if( n < 0 ) return 0;
// else if ( n > maxindex ) return maxindex;
// else return n;
//
// This is not always a clear performance win, but when you have situations where a clamped 
// value is thrashing against a boundary this is a big win. (ie, valid, invalid, valid, invalid, ...)
//
// Note: This code has been run against all possible integers.
//
inline int ClampArrayBounds( int n, unsigned maxindex )
{
	// mask is 0 if less than 4096, 0xFFFFFFFF if greater than
	unsigned int inrangemask = 0xFFFFFFFF + (((unsigned) n) > maxindex );
	unsigned int lessthan0mask = 0xFFFFFFFF + ( n >= 0 );
	
	// If the result was valid, set the result, (otherwise sets zero)
	int result = (inrangemask & n);

	// if the result was out of range or zero.
	result |= ((~inrangemask) & (~lessthan0mask)) & maxindex;

	return result;
}

// halflife is time for value to reach 50%
inline float ExponentialDecay( float halflife, float dt )
{
	// log(0.5) == -0.69314718055994530941723212145818
	return expf( -0.69314718f / halflife * dt);
}

// decayTo is factor the value should decay to in decayTime
inline float ExponentialDecay( float decayTo, float decayTime, float dt )
{
	return expf( logf( decayTo ) / decayTime * dt);
}

// Get the integrated distanced traveled
// decayTo is factor the value should decay to in decayTime
// dt is the time relative to the last velocity update
inline float ExponentialDecayIntegral( float decayTo, float decayTime, float dt  )
{
	return (powf( decayTo, dt / decayTime) * decayTime - decayTime) / logf( decayTo );
}

// hermite basis function for smooth interpolation
// Similar to Gain() above, but very cheap to call
// value should be between 0 & 1 inclusive
inline float SimpleSpline( float value )
{
	float valueSquared = value * value;

	// Nice little ease-in, ease-out spline-like curve
	return (3 * valueSquared - 2 * valueSquared * value);
}

// remaps a value in [startInterval, startInterval+rangeInterval] from linear to
// spline using SimpleSpline
inline float SimpleSplineRemapVal( float val, float A, float B, float C, float D)
{
	if ( A == B )
		return val >= B ? D : C;
	float cVal = (val - A) / (B - A);
	return C + (D - C) * SimpleSpline( cVal );
}

// remaps a value in [startInterval, startInterval+rangeInterval] from linear to
// spline using SimpleSpline
inline float SimpleSplineRemapValClamped( float val, float A, float B, float C, float D )
{
	if ( A == B )
		return val >= B ? D : C;
	float cVal = (val - A) / (B - A);
	cVal = clamp( cVal, 0.0f, 1.0f );
	return C + (D - C) * SimpleSpline( cVal );
}

FORCEINLINE int RoundFloatToInt(float f)
{
#if defined( _X360 )
#ifdef Assert
	Assert( IsFPUControlWordSet() );
#endif
	union
	{
		double flResult;
		int pResult[2];
	};
	flResult = __fctiw( f );
	return pResult[1];
#else // !X360
	int nResult;
#if defined( _WIN32 )
	__asm
	{
		fld f
		fistp nResult
	}
#elif _LINUX
	__asm __volatile__ (
		"fistpl %0;": "=m" (nResult): "t" (f) : "st"
	);
#endif
	return nResult;
#endif
}

FORCEINLINE unsigned char RoundFloatToByte(float f)
{
#if defined( _X360 )
#ifdef Assert
	Assert( IsFPUControlWordSet() );
#endif
	union
	{
		double flResult;
		int pIntResult[2];
		unsigned char pResult[8];
	};
	flResult = __fctiw( f );
#ifdef Assert
	Assert( pIntResult[1] >= 0 && pIntResult[1] <= 255 );
#endif
	return pResult[8];

#else // !X360
	
	int nResult;

#if defined( _WIN32 )
	__asm
	{
		fld f
		fistp nResult
	}
#elif _LINUX
	__asm __volatile__ (
		"fistpl %0;": "=m" (nResult): "t" (f) : "st"
	);
#endif

#ifdef Assert
	Assert( nResult >= 0 && nResult <= 255 );
#endif 
	return nResult;

#endif
}

FORCEINLINE unsigned long RoundFloatToUnsignedLong(float f)
{
#if defined( _X360 )
#ifdef Assert
	Assert( IsFPUControlWordSet() );
#endif
	union
	{
		double flResult;
		int pIntResult[2];
		unsigned long pResult[2];
	};
	flResult = __fctiw( f );
	Assert( pIntResult[1] >= 0 );
	return pResult[1];
#else  // !X360
	
	unsigned char nResult[8];

#if defined( _WIN32 )
	__asm
	{
		fld f
		fistp       qword ptr nResult
	}
#elif _LINUX
	__asm __volatile__ (
		"fistpl %0;": "=m" (nResult): "t" (f) : "st"
	);
#endif

	return *((unsigned long*)nResult);
#endif
}

// Fast, accurate ftol:
FORCEINLINE int Float2Int( float a )
{
#if defined( _X360 )
	union
	{
		double flResult;
		int pResult[2];
	};
	flResult = __fctiwz( a );
	return pResult[1];
#else  // !X360
	
	int RetVal;

#if defined( _WIN32 )
	int CtrlwdHolder;
	int CtrlwdSetter;
	__asm 
	{
		fld    a					// push 'a' onto the FP stack
		fnstcw CtrlwdHolder		// store FPU control word
		movzx  eax, CtrlwdHolder	// move and zero extend word into eax
		and    eax, 0xFFFFF3FF	// set all bits except rounding bits to 1
		or     eax, 0x00000C00	// set rounding mode bits to round towards zero
		mov    CtrlwdSetter, eax	// Prepare to set the rounding mode -- prepare to enter plaid!
		fldcw  CtrlwdSetter		// Entering plaid!
		fistp  RetVal				// Store and converted (to int) result
		fldcw  CtrlwdHolder		// Restore control word
	}
#elif _LINUX
	RetVal = static_cast<int>( a );
#endif

	return RetVal;
#endif
}

// Over 15x faster than: (int)floor(value)
inline int Floor2Int( float a )
{
   int RetVal;

#if defined( _X360 )
	RetVal = (int)floor( a );
#elif defined( _WIN32 )
   int CtrlwdHolder;
   int CtrlwdSetter;
   __asm 
   {
      fld    a					// push 'a' onto the FP stack
      fnstcw CtrlwdHolder		// store FPU control word
      movzx  eax, CtrlwdHolder	// move and zero extend word into eax
      and    eax, 0xFFFFF3FF	// set all bits except rounding bits to 1
      or     eax, 0x00000400	// set rounding mode bits to round down
      mov    CtrlwdSetter, eax	// Prepare to set the rounding mode -- prepare to enter plaid!
      fldcw  CtrlwdSetter		// Entering plaid!
      fistp  RetVal				// Store floored and converted (to int) result
      fldcw  CtrlwdHolder		// Restore control word
   }
#elif _LINUX
	RetVal = static_cast<int>( floor(a) );
#endif

	return RetVal;
}

//-----------------------------------------------------------------------------
// Fast color conversion from float to unsigned char
//-----------------------------------------------------------------------------
FORCEINLINE unsigned char FastFToC( float c )
{
	volatile float dc;

	// ieee trick
	dc = c * 255.0f + (float)(1 << 23);
	
	// return the lsb
#if defined( _X360 )
	return ((unsigned char*)&dc)[3];
#else
	return *(unsigned char*)&dc;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Bound input float to .001 (millisecond) boundary
// Input  : in - 
// Output : inline float
//-----------------------------------------------------------------------------
inline float ClampToMsec( float in )
{
	int msec = Floor2Int( in * 1000.0f + 0.5f );
	return msec / 1000.0f;
}

// Over 15x faster than: (int)ceil(value)
inline int Ceil2Int( float a )
{
   int RetVal;

#if defined( _X360 )
	RetVal = (int)ceil( a );
#elif defined( _WIN32 )
   int CtrlwdHolder;
   int CtrlwdSetter;
   __asm 
   {
      fld    a					// push 'a' onto the FP stack
      fnstcw CtrlwdHolder		// store FPU control word
      movzx  eax, CtrlwdHolder	// move and zero extend word into eax
      and    eax, 0xFFFFF3FF	// set all bits except rounding bits to 1
      or     eax, 0x00000800	// set rounding mode bits to round down
      mov    CtrlwdSetter, eax	// Prepare to set the rounding mode -- prepare to enter plaid!
      fldcw  CtrlwdSetter		// Entering plaid!
      fistp  RetVal				// Store floored and converted (to int) result
      fldcw  CtrlwdHolder		// Restore control word
   }
#elif _LINUX
	RetVal = static_cast<int>( ceil(a) );
#endif

	return RetVal;
}


// Regular signed area of triangle
#define TriArea2D( A, B, C ) \
	( 0.5f * ( ( B.x - A.x ) * ( C.y - A.y ) - ( B.y - A.y ) * ( C.x - A.x ) ) )

// This version doesn't premultiply by 0.5f, so it's the area of the rectangle instead
#define TriArea2DTimesTwo( A, B, C ) \
	( ( ( B.x - A.x ) * ( C.y - A.y ) - ( B.y - A.y ) * ( C.x - A.x ) ) )


// linear (0..4) to screen corrected vertex space (0..1?)
FORCEINLINE float LinearToVertexLight( float f )
{
	extern float lineartovertex[4096];	

	// Gotta clamp before the multiply; could overflow...
	// assume 0..4 range
	int i = RoundFloatToInt( f * 1024.f );

	// Presumably the comman case will be not to clamp, so check that first:
	if( (unsigned)i > 4095 )
	{
		if ( i < 0 )
			i = 0;		// Compare to zero instead of 4095 to save 4 bytes in the instruction stream
		else
			i = 4095;
	}

	return lineartovertex[i];
}


FORCEINLINE unsigned char LinearToLightmap( float f )
{
	extern unsigned char lineartolightmap[4096];	

	// Gotta clamp before the multiply; could overflow...
	int i = RoundFloatToInt( f * 1024.f );	// assume 0..4 range

	// Presumably the comman case will be not to clamp, so check that first:
	if ( (unsigned)i > 4095 )
	{
		if ( i < 0 )
			i = 0;		// Compare to zero instead of 4095 to save 4 bytes in the instruction stream
		else
			i = 4095;
	}

	return lineartolightmap[i];
}

FORCEINLINE void ColorClamp( Vector& color )
{
	float maxc = max( color.x, max( color.y, color.z ) );
	if ( maxc > 1.0f )
	{
		float ooMax = 1.0f / maxc;
		color.x *= ooMax;
		color.y *= ooMax;
		color.z *= ooMax;
	}

	if ( color[0] < 0.f ) color[0] = 0.f;
	if ( color[1] < 0.f ) color[1] = 0.f;
	if ( color[2] < 0.f ) color[2] = 0.f;
}

inline void ColorClampTruncate( Vector& color )
{
	if (color[0] > 1.0f) color[0] = 1.0f; else if (color[0] < 0.0f) color[0] = 0.0f;
	if (color[1] > 1.0f) color[1] = 1.0f; else if (color[1] < 0.0f) color[1] = 0.0f;
	if (color[2] > 1.0f) color[2] = 1.0f; else if (color[2] < 0.0f) color[2] = 0.0f;
}

inline float Approach( float target, float value, float speed )
{
	float delta = target - value;

	if ( delta > speed )
		value += speed;
	else if ( delta < -speed )
		value -= speed;
	else 
		value = target;

	return value;
}



#endif	// MATH_BASE_H

