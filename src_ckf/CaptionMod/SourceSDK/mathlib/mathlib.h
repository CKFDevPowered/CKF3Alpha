//===== Copyright (C) 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
//===========================================================================//

#ifndef MATH_LIB_H
#define MATH_LIB_H
#ifndef vec3_t

#include <math.h>
#include "tier0/basetypes.h"
#include "mathlib/vector.h"
#include "mathlib/vector2d.h"
#include "tier0/dbg.h"

#include "mathlib/math_pfns.h"

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

inline float anglemod(float a)
{
	a = (360.f/65536) * ((int)(a*(65536.f/360.0f)) & 65535);
	return a;
}

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

FORCEINLINE float * UnpackNormal_HEND3N( const unsigned int *pPackedNormal, float *pNormal )
{
	int temp[3];
	temp[0] = ((*pPackedNormal >> 0L) & 0x7ff);
	if ( temp[0] & 0x400 )
	{
		temp[0] = 2048 - temp[0];
	}
	temp[1] = ((*pPackedNormal >> 11L) & 0x7ff);
	if ( temp[1] & 0x400 )
	{
		temp[1] = 2048 - temp[1];
	}
	temp[2] = ((*pPackedNormal >> 22L) & 0x3ff);
	if ( temp[2] & 0x200 )
	{
		temp[2] = 1024 - temp[2];
	}
	pNormal[0] = (float)temp[0] * 1.0f/1023.0f;
	pNormal[1] = (float)temp[1] * 1.0f/1023.0f;
	pNormal[2] = (float)temp[2] * 1.0f/511.0f;
	return pNormal;
}

FORCEINLINE unsigned int * PackNormal_HEND3N( const float *pNormal, unsigned int *pPackedNormal )
{
	int temp[3];

	temp[0] = Float2Int( pNormal[0] * 1023.0f );
	temp[1] = Float2Int( pNormal[1] * 1023.0f );
	temp[2] = Float2Int( pNormal[2] * 511.0f );

	// the normal is out of bounds, determine the source and fix
	// clamping would be even more of a slowdown here
	Assert( temp[0] >= -1023 && temp[0] <= 1023 );
	Assert( temp[1] >= -1023 && temp[1] <= 1023 );
	Assert( temp[2] >= -511 && temp[2] <= 511 );
	
	*pPackedNormal = ( ( temp[2] & 0x3ff ) << 22L ) |
                     ( ( temp[1] & 0x7ff ) << 11L ) |
                     ( ( temp[0] & 0x7ff ) << 0L );
	return pPackedNormal;
}

FORCEINLINE unsigned int * PackNormal_HEND3N( float nx, float ny, float nz, unsigned int *pPackedNormal )
{
	int temp[3];

	temp[0] = Float2Int( nx * 1023.0f );
	temp[1] = Float2Int( ny * 1023.0f );
	temp[2] = Float2Int( nz * 511.0f );

	// the normal is out of bounds, determine the source and fix
	// clamping would be even more of a slowdown here
	Assert( temp[0] >= -1023 && temp[0] <= 1023 );
	Assert( temp[1] >= -1023 && temp[1] <= 1023 );
	Assert( temp[2] >= -511 && temp[2] <= 511 );
	
	*pPackedNormal = ( ( temp[2] & 0x3ff ) << 22L ) |
                     ( ( temp[1] & 0x7ff ) << 11L ) |
                     ( ( temp[0] & 0x7ff ) << 0L );
	return pPackedNormal;
}

FORCEINLINE float * UnpackNormal_SHORT2( const unsigned int *pPackedNormal, float *pNormal, bool bIsTangent = FALSE )
{
	// Unpacks from Jason's 2-short format (fills in a 4th binormal-sign (+1/-1) value, if this is a tangent vector)

	// FIXME: short math is slow on 360 - use ints here instead (bit-twiddle to deal w/ the sign bits)
	short iX = (*pPackedNormal & 0x0000FFFF);
	short iY = (*pPackedNormal & 0xFFFF0000) >> 16;

	float zSign = +1;
	if ( iX < 0 )
	{
		zSign = -1;
		iX    = -iX;
	}
	float tSign = +1;
	if ( iY < 0 )
	{
		tSign = -1;
		iY    = -iY;
	}

	pNormal[0] = ( iX - 16384.0f ) / 16384.0f;
	pNormal[1] = ( iY - 16384.0f ) / 16384.0f;
	pNormal[2] = zSign*sqrtf( 1.0f - ( pNormal[0]*pNormal[0] + pNormal[1]*pNormal[1] ) );
	if ( bIsTangent )
	{
		pNormal[3] = tSign;
	}

	return pNormal;
}

FORCEINLINE unsigned int * PackNormal_SHORT2( float nx, float ny, float nz, unsigned int *pPackedNormal, float binormalSign = +1.0f )
{
	// Pack a vector (ASSUMED TO BE NORMALIZED) into Jason's 4-byte (SHORT2) format.
	// This simply reconstructs Z from X & Y. It uses the sign bits of the X & Y coords
	// to reconstruct the sign of Z and, if this is a tangent vector, the sign of the
	// binormal (this is needed because tangent/binormal vectors are supposed to follow
	// UV gradients, but shaders reconstruct the binormal from the tangent and normal
	// assuming that they form a right-handed basis).

	nx += 1;					// [-1,+1] -> [0,2]
	ny += 1;
	nx *= 16384.0f;				// [ 0, 2] -> [0,32768]
	ny *= 16384.0f;

	// '0' and '32768' values are invalid encodings
	nx = max( nx, 1.0f );		// Make sure there are no zero values
	ny = max( ny, 1.0f );
	nx = min( nx, 32767.0f );	// Make sure there are no 32768 values
	ny = min( ny, 32767.0f );

	if ( nz < 0.0f )
		nx = -nx;				// Set the sign bit for z

	ny *= binormalSign;			// Set the sign bit for the binormal (use when encoding a tangent vector)

	// FIXME: short math is slow on 360 - use ints here instead (bit-twiddle to deal w/ the sign bits), also use Float2Int()
	short sX = (short)nx;		// signed short [1,32767]
	short sY = (short)ny;

	*pPackedNormal = ( sX & 0x0000FFFF ) | ( sY << 16 ); // NOTE: The mask is necessary (if sX is negative and cast to an int...)

	return pPackedNormal;
}

FORCEINLINE unsigned int * PackNormal_SHORT2( const float *pNormal, unsigned int *pPackedNormal, float binormalSign = +1.0f )
{
	return PackNormal_SHORT2( pNormal[0], pNormal[1], pNormal[2], pPackedNormal, binormalSign );
}

// Unpacks a UBYTE4 normal (for a tangent, the result's fourth component receives the binormal 'sign')
FORCEINLINE float * UnpackNormal_UBYTE4( const unsigned int *pPackedNormal, float *pNormal, bool bIsTangent = FALSE )
{
	unsigned char cX, cY;
	if ( bIsTangent )
	{
		cX = *pPackedNormal >> 16;					// Unpack Z
		cY = *pPackedNormal >> 24;					// Unpack W
	}
	else
	{
		cX = *pPackedNormal >>  0;					// Unpack X
		cY = *pPackedNormal >>  8;					// Unpack Y
	}

	float x = cX - 128.0f;
	float y = cY - 128.0f;
	float z;

	float zSignBit = x < 0 ? 1.0f : 0.0f;			// z and t negative bits (like slt asm instruction)
	float tSignBit = y < 0 ? 1.0f : 0.0f;
	float zSign    = -( 2*zSignBit - 1 );			// z and t signs
	float tSign    = -( 2*tSignBit - 1 );

	x = x*zSign - zSignBit;							// 0..127
	y = y*tSign - tSignBit;
	x = x - 64;										// -64..63
	y = y - 64;

	float xSignBit = x < 0 ? 1.0f : 0.0f;	// x and y negative bits (like slt asm instruction)
	float ySignBit = y < 0 ? 1.0f : 0.0f;
	float xSign    = -( 2*xSignBit - 1 );			// x and y signs
	float ySign    = -( 2*ySignBit - 1 );

	x = ( x*xSign - xSignBit ) / 63.0f;				// 0..1 range
	y = ( y*ySign - ySignBit ) / 63.0f;
	z = 1.0f - x - y;

	float oolen	 = 1.0f / sqrt( x*x + y*y + z*z );	// Normalize and
	x			*= oolen * xSign;					// Recover signs
	y			*= oolen * ySign;
	z			*= oolen * zSign;

	pNormal[0] = x;
	pNormal[1] = y;
	pNormal[2] = z;
	if ( bIsTangent )
	{
		pNormal[3] = tSign;
	}

	return pNormal;
}

//////////////////////////////////////////////////////////////////////////////
// See: http://www.oroboro.com/rafael/docserv.php/index/programming/article/unitv2
//
// UBYTE4 encoding, using per-octant projection onto x+y+z=1
// Assume input vector is already unit length
//
// binormalSign specifies 'sign' of binormal, stored in t sign bit of tangent
// (lets the shader know whether norm/tan/bin form a right-handed basis)
//
// bIsTangent is used to specify which WORD of the output to store the data
// The expected usage is to call once with the normal and once with
// the tangent and binormal sign flag, bitwise OR'ing the returned DWORDs
FORCEINLINE unsigned int * PackNormal_UBYTE4( float nx, float ny, float nz, unsigned int *pPackedNormal, bool bIsTangent = false, float binormalSign = +1.0f )
{
	float xSign = nx < 0.0f ? -1.0f : 1.0f;			// -1 or 1 sign
	float ySign = ny < 0.0f ? -1.0f : 1.0f;
	float zSign = nz < 0.0f ? -1.0f : 1.0f;
	float tSign = binormalSign;
	Assert( ( binormalSign == +1.0f ) || ( binormalSign == -1.0f ) );

	float xSignBit = 0.5f*( 1 - xSign );			// [-1,+1] -> [1,0]
	float ySignBit = 0.5f*( 1 - ySign );			// 1 is negative bit (like slt instruction)
	float zSignBit = 0.5f*( 1 - zSign );
	float tSignBit = 0.5f*( 1 - binormalSign );		

	float absX = xSign*nx;							// 0..1 range (abs)
	float absY = ySign*ny;
	float absZ = zSign*nz;

	float xbits = absX / ( absX + absY + absZ );	// Project onto x+y+z=1 plane
	float ybits = absY / ( absX + absY + absZ );

	xbits *= 63;									// 0..63
	ybits *= 63;

	xbits  = xbits * xSign - xSignBit;				// -64..63 range
	ybits  = ybits * ySign - ySignBit;
	xbits += 64.0f;									// 0..127 range
	ybits += 64.0f;

	xbits  = xbits * zSign - zSignBit;				// Negate based on z and t
	ybits  = ybits * tSign - tSignBit;				// -128..127 range

	xbits += 128.0f;								// 0..255 range
	ybits += 128.0f;

	unsigned char cX = (unsigned char) xbits;
	unsigned char cY = (unsigned char) ybits;

	if ( !bIsTangent )
		*pPackedNormal = (cX <<  0) | (cY <<  8);	// xy for normal
	else						   
		*pPackedNormal = (cX << 16) | (cY << 24);	// zw for tangent

	return pPackedNormal;
}

FORCEINLINE unsigned int * PackNormal_UBYTE4( const float *pNormal, unsigned int *pPackedNormal, bool bIsTangent = false, float binormalSign = +1.0f )
{
	return PackNormal_UBYTE4( pNormal[0], pNormal[1], pNormal[2], pPackedNormal, bIsTangent, binormalSign );
}


#endif
#endif	// MATH_BASE_H

