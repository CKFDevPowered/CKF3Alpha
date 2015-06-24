//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: String Tools
//
//===========================================================================//

// These are redefined in the project settings to prevent anyone from using them.
// We in this module are of a higher caste and thus are privileged in their use.
#ifdef strncpy
	#undef strncpy
#endif

#ifdef _snprintf
	#undef _snprintf
#endif

#if defined( sprintf )
	#undef sprintf
#endif

#if defined( vsprintf )
	#undef vsprintf
#endif

#ifdef _vsnprintf
#ifdef _WIN32
	#undef _vsnprintf
#endif
#endif

#ifdef vsnprintf
#ifndef _WIN32
	#undef vsnprintf
#endif
#endif

#if defined( strcat )
	#undef strcat
#endif

#ifdef strncat
	#undef strncat
#endif

// NOTE: I have to include stdio + stdarg first so vsnprintf gets compiled in
#include <stdio.h>
#include <stdarg.h>

#ifdef POSIX
#include <iconv.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#define _getcwd getcwd
#elif _WIN32
#include <direct.h>
#if !defined( _X360 )
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#endif

#ifdef _WIN32
#ifndef CP_UTF8
#define CP_UTF8 65001
#endif
#endif
#include "tier0/dbg.h"
#include "tier1/strtools.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "tier0/basetypes.h"
#include "tier1/utldict.h"
#if defined( _X360 )
#include "xbox/xbox_win32stubs.h"
#endif
#include "tier0/memdbgon.h"

static int FastToLower( char c )
{
	int i = (unsigned char) c;
	if ( i < 0x80 )
	{
		// Brutally fast branchless ASCII tolower():
		i += (((('A'-1) - i) & (i - ('Z'+1))) >> 26) & 0x20;
	}
	else
	{
		i += isupper( i ) ? 0x20 : 0;
	}
	return i;
}

void _V_memset (const char* file, int line, void *dest, int fill, int count)
{
	Assert( count >= 0 );
	AssertValidWritePtr( dest, count );

	memset(dest,fill,count);
}

void _V_memcpy (const char* file, int line, void *dest, const void *src, int count)
{
	Assert( count >= 0 );
	AssertValidReadPtr( src, count );
	AssertValidWritePtr( dest, count );

	memcpy( dest, src, count );
}

void _V_memmove(const char* file, int line, void *dest, const void *src, int count)
{
	Assert( count >= 0 );
	AssertValidReadPtr( src, count );
	AssertValidWritePtr( dest, count );

	memmove( dest, src, count );
}

int _V_memcmp (const char* file, int line, const void *m1, const void *m2, int count)
{
	Assert( count >= 0 );
	AssertValidReadPtr( m1, count );
	AssertValidReadPtr( m2, count );

	return memcmp( m1, m2, count );
}

int	_V_strlen(const char* file, int line, const char *str)
{
	AssertValidStringPtr(str);
	return strlen( str );
}

void _V_strcpy (const char* file, int line, char *dest, const char *src)
{
	AssertValidWritePtr(dest);
	AssertValidStringPtr(src);

	strcpy( dest, src );
}

int	_V_wcslen(const char* file, int line, const wchar_t *pwch)
{
	return wcslen( pwch );
}

char *_V_strrchr(const char* file, int line, const char *s, char c)
{
	AssertValidStringPtr( s );
    int len = V_strlen(s);
    s += len;
    while (len--)
	if (*--s == c) return (char *)s;
    return 0;
}

int _V_strcmp (const char* file, int line, const char *s1, const char *s2)
{
	AssertValidStringPtr( s1 );
	AssertValidStringPtr( s2 );

	return strcmp( s1, s2 );
}

int _V_wcscmp (const char* file, int line, const wchar_t *s1, const wchar_t *s2)
{
	AssertValidReadPtr( s1 );
	AssertValidReadPtr( s2 );

	while ( *s1 == *s2 )
	{
		if ( !*s1 )
			return 0;			// strings are equal

		s1++;
		s2++;
	}

	return *s1 > *s2 ? 1 : -1;	// strings not equal
}


char *_V_strstr(const char* file, int line,  const char *s1, const char *search )
{
	AssertValidStringPtr( s1 );
	AssertValidStringPtr( search );

#if defined( _X360 )
	return (char *)strstr( (char *)s1, search );
#else
	return (char *)strstr( s1, search );
#endif
}

wchar_t *_V_wcsupr (const char* file, int line, wchar_t *start)
{
	return _wcsupr( start );
}


wchar_t *_V_wcslower (const char* file, int line, wchar_t *start)
{
	return _wcslwr(start);
}



char *V_strupr( char *start )
{
	unsigned char *str = (unsigned char*)start;
	while( *str )
	{
		if ( (unsigned char)(*str - 'a') <= ('z' - 'a') )
			*str -= 'a' - 'A';
		else if ( (unsigned char)*str >= 0x80 ) // non-ascii, fall back to CRT
			*str = toupper( *str );
		str++;
	}
	return start;
}

char *V_strlower( char *start )
{
	unsigned char *str = (unsigned char*)start;
	while( *str )
	{
		if ( (unsigned char)(*str - 'A') <= ('Z' - 'A') )
			*str += 'a' - 'A';
		else if ( (unsigned char)*str >= 0x80 ) // non-ascii, fall back to CRT
			*str = tolower( *str );
		str++;
	}
	return start;
}

char *V_strnlwr(char *s, size_t count)
{
	// Assert( count >= 0 ); tautology since size_t is unsigned
	AssertValidStringPtr( s, count );

	char* pRet = s;
	if ( !s || !count )
		return s;

	while ( -- count > 0 )
	{
		if ( !*s )
			return pRet; // reached end of string

		*s = tolower( *s );
		++s;
	}

	*s = 0; // null-terminate original string at "count-1"
	return pRet;
}

int V_stricmp( const char *str1, const char *str2 )
{
	// It is not uncommon to compare a string to itself. See
	// VPanelWrapper::GetPanel which does this a lot. Since stricmp
	// is expensive and pointer comparison is cheap, this simple test
	// can save a lot of cycles, and cache pollution.
	if ( str1 == str2 )
	{
		return 0;
	}
	const unsigned char *s1 = (const unsigned char*)str1;
	const unsigned char *s2 = (const unsigned char*)str2;
	for ( ; *s1; ++s1, ++s2 )
	{
		if ( *s1 != *s2 )
		{
			// in ascii char set, lowercase = uppercase | 0x20
			unsigned char c1 = *s1 | 0x20;
			unsigned char c2 = *s2 | 0x20;
			if ( c1 != c2 || (unsigned char)(c1 - 'a') > ('z' - 'a') )
			{
				// if non-ascii mismatch, fall back to CRT for locale
				if ( (c1 | c2) >= 0x80 ) return stricmp( (const char*)s1, (const char*)s2 );
				// ascii mismatch. only use the | 0x20 value if alphabetic.
				if ((unsigned char)(c1 - 'a') > ('z' - 'a')) c1 = *s1;
				if ((unsigned char)(c2 - 'a') > ('z' - 'a')) c2 = *s2;
				return c1 > c2 ? 1 : -1;
			}
		}
	}
	return *s2 ? -1 : 0;
}

int V_strnicmp( const char *str1, const char *str2, int n )
{
	const unsigned char *s1 = (const unsigned char*)str1;
	const unsigned char *s2 = (const unsigned char*)str2;
	for ( ; n > 0 && *s1; --n, ++s1, ++s2 )
	{
		if ( *s1 != *s2 )
		{
			// in ascii char set, lowercase = uppercase | 0x20
			unsigned char c1 = *s1 | 0x20;
			unsigned char c2 = *s2 | 0x20;
			if ( c1 != c2 || (unsigned char)(c1 - 'a') > ('z' - 'a') )
			{
				// if non-ascii mismatch, fall back to CRT for locale
				if ( (c1 | c2) >= 0x80 ) return strnicmp( (const char*)s1, (const char*)s2, n );
				// ascii mismatch. only use the | 0x20 value if alphabetic.
				if ((unsigned char)(c1 - 'a') > ('z' - 'a')) c1 = *s1;
				if ((unsigned char)(c2 - 'a') > ('z' - 'a')) c2 = *s2;
				return c1 > c2 ? 1 : -1;
			}
		}
	}
	return (n > 0 && *s2) ? -1 : 0;
}

int V_strncmp( const char *s1, const char *s2, int count )
{
	Assert( count >= 0 );
	AssertValidStringPtr( s1, count );
	AssertValidStringPtr( s2, count );

	while ( count > 0 )
	{
		if ( *s1 != *s2 )
			return (unsigned char)*s1 < (unsigned char)*s2 ? -1 : 1; // string different
		if ( *s1 == '\0' )
			return 0; // null terminator hit - strings the same
		s1++;
		s2++;
		count--;
	}

	return 0; // count characters compared the same
}


const char *StringAfterPrefix( const char *str, const char *prefix )
{
	AssertValidStringPtr( str );
	AssertValidStringPtr( prefix );
	do
	{
		if ( !*prefix )
			return str;
	}
	while ( FastToLower( *str++ ) == FastToLower( *prefix++ ) );
	return NULL;
}

const char *StringAfterPrefixCaseSensitive( const char *str, const char *prefix )
{
	AssertValidStringPtr( str );
	AssertValidStringPtr( prefix );
	do
	{
		if ( !*prefix )
			return str;
	}
	while ( *str++ == *prefix++ );
	return NULL;
}


int64 V_atoi64( const char *str )
{
	AssertValidStringPtr( str );

	int64             val;
	int64             sign;
	int64             c;
	
	Assert( str );
	if (*str == '-')
	{
		sign = -1;
		str++;
	}
	else
		sign = 1;
		
	val = 0;

//
// check for hex
//
	if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X') )
	{
		str += 2;
		while (1)
		{
			c = *str++;
			if (c >= '0' && c <= '9')
				val = (val<<4) + c - '0';
			else if (c >= 'a' && c <= 'f')
				val = (val<<4) + c - 'a' + 10;
			else if (c >= 'A' && c <= 'F')
				val = (val<<4) + c - 'A' + 10;
			else
				return val*sign;
		}
	}
	
//
// check for character
//
	if (str[0] == '\'')
	{
		return sign * str[1];
	}
	
//
// assume decimal
//
	while (1)
	{
		c = *str++;
		if (c <'0' || c > '9')
			return val*sign;
		val = val*10 + c - '0';
	}
	
	return 0;
}

uint64 V_atoui64( const char *str )
{
	AssertValidStringPtr( str );

	uint64             val;
	uint64             c;

	Assert( str );

	val = 0;

	//
	// check for hex
	//
	if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X') )
	{
		str += 2;
		while (1)
		{
			c = *str++;
			if (c >= '0' && c <= '9')
				val = (val<<4) + c - '0';
			else if (c >= 'a' && c <= 'f')
				val = (val<<4) + c - 'a' + 10;
			else if (c >= 'A' && c <= 'F')
				val = (val<<4) + c - 'A' + 10;
			else
				return val;
		}
	}

	//
	// check for character
	//
	if (str[0] == '\'')
	{
		return str[1];
	}

	//
	// assume decimal
	//
	while (1)
	{
		c = *str++;
		if (c <'0' || c > '9')
			return val;
		val = val*10 + c - '0';
	}

	return 0;
}

int V_atoi( const char *str )
{ 
	return (int)V_atoi64( str );
}

float V_atof (const char *str)
{
	AssertValidStringPtr( str );
	double			val;
	int             sign;
	int             c;
	int             decimal, total;

	if (*str == '-')
	{
		sign = -1;
		str++;
	}
	else if (*str == '+')
	{
		sign = 1;
		str++;
	}
	else
	{
		sign = 1;
	}

	val = 0;

	//
	// check for hex
	//
	if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X') )
	{
		str += 2;
		while (1)
		{
			c = *str++;
			if (c >= '0' && c <= '9')
				val = (val*16) + c - '0';
			else if (c >= 'a' && c <= 'f')
				val = (val*16) + c - 'a' + 10;
			else if (c >= 'A' && c <= 'F')
				val = (val*16) + c - 'A' + 10;
			else
				return val*sign;
		}
	}

	//
	// check for character
	//
	if (str[0] == '\'')
	{
		return sign * str[1];
	}

	//
	// assume decimal
	//
	decimal = -1;
	total = 0;
	int exponent = 0;
	while (1)
	{
		c = *str++;
		if (c == '.')
		{
			if ( decimal != -1 )
			{
				break;
			}

			decimal = total;
			continue;
		}
		if (c <'0' || c > '9')
		{
			if ( c == 'e' || c == 'E' )
			{
				exponent = V_atoi(str);
			}
			break;
		}
		val = val*10 + c - '0';
		total++;
	}

	if ( exponent != 0 )
	{
		val *= pow( 10.0, exponent );
	}
	if (decimal == -1)
		return val*sign;
	while (total > decimal)
	{
		val /= 10;
		total--;
	}

	return val*sign;
}

//-----------------------------------------------------------------------------
// Normalizes a float string in place.  
//
// (removes leading zeros, trailing zeros after the decimal point, and the decimal point itself where possible)
//-----------------------------------------------------------------------------
void V_normalizeFloatString( char* pFloat )
{
	// If we have a decimal point, remove trailing zeroes:
	if( strchr( pFloat,'.' ) )
	{
		int len = V_strlen(pFloat);

		while( len > 1 && pFloat[len - 1] == '0' )
		{
			pFloat[len - 1] = '\0';
			len--;
		}

		if( len > 1 && pFloat[ len - 1 ] == '.' )
		{
			pFloat[len - 1] = '\0';
			len--;
		}
	}

	// TODO: Strip leading zeros

}


//-----------------------------------------------------------------------------
// Finds a string in another string with a case insensitive test
//-----------------------------------------------------------------------------
char const* V_stristr( char const* pStr, char const* pSearch )
{
	AssertValidStringPtr(pStr);
	AssertValidStringPtr(pSearch);

	if (!pStr || !pSearch) 
		return 0;

	char const* pLetter = pStr;

	// Check the entire string
	while (*pLetter != 0)
	{
		// Skip over non-matches
		if (FastToLower((unsigned char)*pLetter) == FastToLower((unsigned char)*pSearch))
		{
			// Check for match
			char const* pMatch = pLetter + 1;
			char const* pTest = pSearch + 1;
			while (*pTest != 0)
			{
				// We've run off the end; don't bother.
				if (*pMatch == 0)
					return 0;

				if (FastToLower((unsigned char)*pMatch) != FastToLower((unsigned char)*pTest))
					break;

				++pMatch;
				++pTest;
			}

			// Found a match!
			if (*pTest == 0)
				return pLetter;
		}

		++pLetter;
	}

	return 0;
}

char* V_stristr( char* pStr, char const* pSearch )
{
	AssertValidStringPtr( pStr );
	AssertValidStringPtr( pSearch );

	return (char*)V_stristr( (char const*)pStr, pSearch );
}

//-----------------------------------------------------------------------------
// Finds a string in another string with a case insensitive test w/ length validation
//-----------------------------------------------------------------------------

char const* V_strnistr( char const* pStr, char const* pSearch, int n )
{
	AssertValidStringPtr(pStr);
	AssertValidStringPtr(pSearch);

	if (!pStr || !pSearch) 
		return 0;

	char const* pLetter = pStr;

	// Check the entire string
	while (*pLetter != 0)
	{
		if ( n <= 0 )
			return 0;

		// Skip over non-matches
		if (FastToLower(*pLetter) == FastToLower(*pSearch))
		{
			int n1 = n - 1;

			// Check for match
			char const* pMatch = pLetter + 1;
			char const* pTest = pSearch + 1;
			while (*pTest != 0)
			{
				if ( n1 <= 0 )
					return 0;

				// We've run off the end; don't bother.
				if (*pMatch == 0)
					return 0;

				if (FastToLower(*pMatch) != FastToLower(*pTest))
					break;

				++pMatch;
				++pTest;
				--n1;
			}

			// Found a match!
			if (*pTest == 0)
				return pLetter;
		}

		++pLetter;
		--n;
	}

	return 0;
}

const char* V_strnchr( const char* pStr, char c, int n )
{
	char const* pLetter = pStr;
	char const* pLast = pStr + n;

	// Check the entire string
	while ( (pLetter < pLast) && (*pLetter != 0) )
	{
		if (*pLetter == c)
			return pLetter;
		++pLetter;
	}
	return NULL;
}

void V_strncpy( char *pDest, char const *pSrc, int maxLen )
{
	Assert( maxLen >= sizeof( *pDest ) );
	AssertValidWritePtr( pDest, maxLen );
	AssertValidStringPtr( pSrc );

	strncpy( pDest, pSrc, maxLen );
	if ( maxLen > 0 )
	{
		pDest[maxLen-1] = 0;
	}
}

// warning C6053: Call to 'wcsncpy' might not zero-terminate string 'pDest'
// warning C6059: Incorrect length parameter in call to 'strncat'. Pass the number of remaining characters, not the buffer size of 'argument 1'
// warning C6386: Buffer overrun: accessing 'argument 1', the writable size is 'destBufferSize' bytes, but '1000' bytes might be written
// These warnings were investigated through code inspection and writing of tests and they are
// believed to all be spurious.
#ifdef _PREFAST_
#pragma warning( push )
#pragma warning( disable : 6053 6059 6386 )
#endif

void V_wcsncpy( wchar_t *pDest, wchar_t const *pSrc, int maxLenInBytes )
{
	Assert( maxLenInBytes >= sizeof( *pDest ) );
	AssertValidWritePtr( pDest, maxLenInBytes );
	AssertValidReadPtr( pSrc );

	int maxLen = maxLenInBytes / sizeof(wchar_t);

	wcsncpy( pDest, pSrc, maxLen );
	if( maxLen )
	{
		pDest[maxLen-1] = 0;
	}
}



int V_snwprintf( wchar_t *pDest, int maxLen, const wchar_t *pFormat, ... )
{
	Assert( maxLen > 0 );
	AssertValidWritePtr( pDest, maxLen );
	AssertValidReadPtr( pFormat );

	va_list marker;

	va_start( marker, pFormat );
#ifdef _WIN32
	int len = _vsnwprintf( pDest, maxLen, pFormat, marker );
#elif POSIX
	int len = vswprintf( pDest, maxLen, pFormat, marker );
#else
#error "define vsnwprintf type."
#endif
	va_end( marker );

	// Len > maxLen represents an overflow on POSIX, < 0 is an overflow on windows
	if( len < 0 || len >= maxLen )
	{
		len = maxLen;
		pDest[maxLen-1] = 0;
	}
	
	return len;
}


int V_vsnwprintf( wchar_t *pDest, int maxLen, const wchar_t *pFormat, va_list params )
{
	Assert( maxLen > 0 );

#ifdef _WIN32
	int len = _vsnwprintf( pDest, maxLen, pFormat, params );
#elif POSIX
	int len = vswprintf( pDest, maxLen, pFormat, params );
#else
#error "define vsnwprintf type."
#endif

	// Len < 0 represents an overflow
	// Len == maxLen represents exactly fitting with no NULL termination
	// Len >= maxLen represents overflow on POSIX
	if ( len < 0 || len >= maxLen )
	{
		len = maxLen;
		pDest[maxLen-1] = 0;
	}

	return len;
}


int V_snprintf( char *pDest, int maxLen, char const *pFormat, ... )
{
	Assert( maxLen > 0 );
	AssertValidWritePtr( pDest, maxLen );
	AssertValidStringPtr( pFormat );

	va_list marker;

	va_start( marker, pFormat );
#ifdef _WIN32
	int len = _vsnprintf( pDest, maxLen, pFormat, marker );
#elif POSIX
	int len = vsnprintf( pDest, maxLen, pFormat, marker );
#else
	#error "define vsnprintf type."
#endif
	va_end( marker );

	// Len > maxLen represents an overflow on POSIX, < 0 is an overflow on windows
	if( len < 0 || len >= maxLen )
	{
		len = maxLen;
		pDest[maxLen-1] = 0;
	}

	return len;
}


int V_vsnprintf( char *pDest, int maxLen, char const *pFormat, va_list params )
{
	Assert( maxLen > 0 );
	AssertValidWritePtr( pDest, maxLen );
	AssertValidStringPtr( pFormat );

	int len = _vsnprintf( pDest, maxLen, pFormat, params );

	// Len > maxLen represents an overflow on POSIX, < 0 is an overflow on windows
	if( len < 0 || len >= maxLen )
	{
		len = maxLen;
		pDest[maxLen-1] = 0;
	}

	return len;
}


int V_vsnprintfRet( char *pDest, int maxLen, const char *pFormat, va_list params, bool *pbTruncated )
{
	Assert( maxLen > 0 );
	AssertValidWritePtr( pDest, maxLen );
	AssertValidStringPtr( pFormat );

	int len = _vsnprintf( pDest, maxLen, pFormat, params );

	if ( pbTruncated )
	{
		*pbTruncated = ( len < 0 || len >= maxLen );
	}

	if	( len < 0 || len >= maxLen )
	{
		len = maxLen;
		pDest[maxLen-1] = 0;
	}

	return len;
}



//-----------------------------------------------------------------------------
// Purpose: If COPY_ALL_CHARACTERS == max_chars_to_copy then we try to add the whole pSrc to the end of pDest, otherwise
//  we copy only as many characters as are specified in max_chars_to_copy (or the # of characters in pSrc if thats's less).
// Input  : *pDest - destination buffer
//			*pSrc - string to append
//			destBufferSize - sizeof the buffer pointed to by pDest
//			max_chars_to_copy - COPY_ALL_CHARACTERS in pSrc or max # to copy
// Output : char * the copied buffer
//-----------------------------------------------------------------------------
char *V_strncat(char *pDest, const char *pSrc, size_t destBufferSize, int max_chars_to_copy )
{
	size_t charstocopy = (size_t)0;

	Assert( (ptrdiff_t)destBufferSize >= 0 );
	AssertValidStringPtr( pDest);
	AssertValidStringPtr( pSrc );
	
	size_t len = strlen(pDest);
	size_t srclen = strlen( pSrc );
	if ( max_chars_to_copy <= COPY_ALL_CHARACTERS )
	{
		charstocopy = srclen;
	}
	else
	{
		charstocopy = (size_t)min( max_chars_to_copy, (int)srclen );
	}

	if ( len + charstocopy >= destBufferSize )
	{
		charstocopy = destBufferSize - len - 1;
	}

	if ( (int)charstocopy <= 0 )
	{
		return pDest;
	}

	char *pOut = strncat( pDest, pSrc, charstocopy );
	return pOut;
}

wchar_t *V_wcsncat( wchar_t *pDest, const wchar_t *pSrc, size_t cchDest, int max_chars_to_copy )
{
	size_t charstocopy = (size_t)0;

	Assert( (ptrdiff_t)cchDest >= 0 );
	
	size_t len = wcslen(pDest);
	size_t srclen = wcslen( pSrc );
	if ( max_chars_to_copy <= COPY_ALL_CHARACTERS )
	{
		charstocopy = srclen;
	}
	else
	{
		charstocopy = (size_t)min( max_chars_to_copy, (int)srclen );
	}

	if ( len + charstocopy >= cchDest )
	{
		charstocopy = cchDest - len - 1;
	}

	if ( (int)charstocopy <= 0 )
	{
		return pDest;
	}

	wchar_t *pOut = wcsncat( pDest, pSrc, charstocopy );
	return pOut;
}



//-----------------------------------------------------------------------------
// Purpose: Converts value into x.xx MB/ x.xx KB, x.xx bytes format, including commas
// Input  : value - 
//			2 - 
//			false - 
// Output : char
//-----------------------------------------------------------------------------
#define NUM_PRETIFYMEM_BUFFERS 8
char *V_pretifymem( float value, int digitsafterdecimal /*= 2*/, bool usebinaryonek /*= false*/ )
{
	static char output[ NUM_PRETIFYMEM_BUFFERS ][ 32 ];
	static int  current;

	float		onekb = usebinaryonek ? 1024.0f : 1000.0f;
	float		onemb = onekb * onekb;

	char *out = output[ current ];
	current = ( current + 1 ) & ( NUM_PRETIFYMEM_BUFFERS -1 );

	char suffix[ 8 ];

	// First figure out which bin to use
	if ( value > onemb )
	{
		value /= onemb;
		V_snprintf( suffix, sizeof( suffix ), " MB" );
	}
	else if ( value > onekb )
	{
		value /= onekb;
		V_snprintf( suffix, sizeof( suffix ), " KB" );
	}
	else
	{
		V_snprintf( suffix, sizeof( suffix ), " bytes" );
	}

	char val[ 32 ];

	// Clamp to >= 0
	digitsafterdecimal = max( digitsafterdecimal, 0 );

	// If it's basically integral, don't do any decimals
	if ( FloatMakePositive( value - (int)value ) < 0.00001 )
	{
		V_snprintf( val, sizeof( val ), "%i%s", (int)value, suffix );
	}
	else
	{
		char fmt[ 32 ];

		// Otherwise, create a format string for the decimals
		V_snprintf( fmt, sizeof( fmt ), "%%.%if%s", digitsafterdecimal, suffix );
		V_snprintf( val, sizeof( val ), fmt, value );
	}

	// Copy from in to out
	char *i = val;
	char *o = out;

	// Search for decimal or if it was integral, find the space after the raw number
	char *dot = strstr( i, "." );
	if ( !dot )
	{
		dot = strstr( i, " " );
	}

	// Compute position of dot
	int pos = dot - i;
	// Don't put a comma if it's <= 3 long
	pos -= 3;

	while ( *i )
	{
		// If pos is still valid then insert a comma every third digit, except if we would be
		//  putting one in the first spot
		if ( pos >= 0 && !( pos % 3 ) )
		{
			// Never in first spot
			if ( o != out )
			{
				*o++ = ',';
			}
		}

		// Count down comma position
		pos--;

		// Copy rest of data as normal
		*o++ = *i++;
	}

	// Terminate
	*o = 0;

	return out;
}

//-----------------------------------------------------------------------------
// Purpose: Returns a string representation of an integer with commas
//			separating the 1000s (ie, 37,426,421)
// Input  : value -		Value to convert
// Output : Pointer to a static buffer containing the output
//-----------------------------------------------------------------------------
#define NUM_PRETIFYNUM_BUFFERS 8 // Must be a power of two
char *V_pretifynum( int64 inputValue )
{
	static char output[ NUM_PRETIFYMEM_BUFFERS ][ 32 ];
	static int  current;

	// Point to the output buffer.
	char * const out = output[ current ];
	// Track the output buffer end for easy calculation of bytes-remaining.
	const char* const outEnd = out + sizeof( output[ current ] );

	// Point to the current output location in the output buffer.
	char *pchRender = out;
	// Move to the next output pointer.
	current = ( current + 1 ) & ( NUM_PRETIFYMEM_BUFFERS -1 );

	*out = 0;

	// In order to handle the most-negative int64 we need to negate it
	// into a uint64.
	uint64 value;
	// Render the leading minus sign, if necessary
	if ( inputValue < 0 )
	{
		V_snprintf( pchRender, 32, "-" );
		value = (uint64)-inputValue;
		// Advance our output pointer.
		pchRender += V_strlen( pchRender );
	}
	else
	{
		value = (uint64)inputValue;
	}

	// Now let's find out how big our number is. The largest number we can fit
	// into 63 bits is about 9.2e18. So, there could potentially be six
	// three-digit groups.

	// We need the initial value of 'divisor' to be big enough to divide our
	// number down to 1-999 range.
	uint64 divisor = 1;
	// Loop more than six times to avoid integer overflow.
	for ( int i = 0; i < 6; ++i )
	{
		// If our divisor is already big enough then stop.
		if ( value < divisor * 1000 )
			break;

		divisor *= 1000;
	}

	// Print the leading batch of one to three digits.
	int toPrint = value / divisor;
	V_snprintf( pchRender, outEnd - pchRender, "%d", toPrint );

	for (;;)
	{
		// Advance our output pointer.
		pchRender += V_strlen( pchRender );
		// Adjust our value to be printed and our divisor.
		value -= toPrint * divisor;
		divisor /= 1000;
		if ( !divisor )
			break;

		// The remaining blocks of digits always include a comma and three digits.
		toPrint = value / divisor;
		V_snprintf( pchRender, outEnd - pchRender, ",%03d", toPrint );
	}

	return out;
}


//-----------------------------------------------------------------------------
// Purpose: returns true if a wide character is a "mean" space; that is,
//			if it is technically a space or punctuation, but causes disruptive
//			behavior when used in names, web pages, chat windows, etc.
//
//			characters in this set are removed from the beginning and/or end of strings
//			by Q_AggressiveStripPrecedingAndTrailingWhitespaceW() 
//-----------------------------------------------------------------------------
bool Q_IsMeanSpaceW( wchar_t wch )
{
	bool bIsMean = false;

	switch ( wch )
	{
	case L'\x0082':	  // BREAK PERMITTED HERE
	case L'\x0083':	  // NO BREAK PERMITTED HERE
	case L'\x00A0':	  // NO-BREAK SPACE
	case L'\x034F':   // COMBINING GRAPHEME JOINER
	case L'\x2000':   // EN QUAD
	case L'\x2001':   // EM QUAD
	case L'\x2002':   // EN SPACE
	case L'\x2003':   // EM SPACE
	case L'\x2004':   // THICK SPACE
	case L'\x2005':   // MID SPACE
	case L'\x2006':   // SIX SPACE
	case L'\x2007':   // figure space
	case L'\x2008':   // PUNCTUATION SPACE
	case L'\x2009':   // THIN SPACE
	case L'\x200A':   // HAIR SPACE
	case L'\x200B':   // ZERO-WIDTH SPACE
	case L'\x200C':   // ZERO-WIDTH NON-JOINER
	case L'\x200D':   // ZERO WIDTH JOINER
	case L'\x2028':   // LINE SEPARATOR
	case L'\x2029':   // PARAGRAPH SEPARATOR
	case L'\x202F':   // NARROW NO-BREAK SPACE
	case L'\x2060':   // word joiner
	case L'\xFEFF':   // ZERO-WIDTH NO BREAK SPACE
	case L'\xFFFC':   // OBJECT REPLACEMENT CHARACTER
		bIsMean = true;
		break;
	}

	return bIsMean;
}


//-----------------------------------------------------------------------------
// Purpose: strips trailing whitespace; returns pointer inside string just past
// any leading whitespace.
//
// bAggresive = true causes this function to also check for "mean" spaces,
// which we don't want in persona names or chat strings as they're disruptive
// to the user experience.
//-----------------------------------------------------------------------------
static wchar_t *StripWhitespaceWorker( int cchLength, wchar_t *pwch, bool *pbStrippedWhitespace, bool bAggressive )
{
	// walk backwards from the end of the string, killing any whitespace
	*pbStrippedWhitespace = false;

	wchar_t *pwchEnd = pwch + cchLength;
	while ( --pwchEnd >= pwch )
	{
		if ( !iswspace( *pwchEnd ) && ( !bAggressive || !Q_IsMeanSpaceW( *pwchEnd ) ) )
			break;

		*pwchEnd = 0;
		*pbStrippedWhitespace = true;
	}

	// walk forward in the string
	while ( pwch < pwchEnd )
	{
		if ( !iswspace( *pwch ) )
			break;

		*pbStrippedWhitespace = true;
		pwch++;
	}

	return pwch;
}


//-----------------------------------------------------------------------------
// Purpose: strips leading and trailing whitespace
//-----------------------------------------------------------------------------
bool Q_StripPrecedingAndTrailingWhitespaceW( wchar_t *pwch )
{
	// duplicate on stack
	int cch = Q_wcslen( pwch );
	int cubDest = ( cch + 1 ) * sizeof( wchar_t );
	wchar_t *pwchT = (wchar_t *)stackalloc( cubDest );
	Q_wcsncpy( pwchT, pwch, cubDest );

	bool bStrippedWhitespace = false;
	pwchT = StripWhitespaceWorker( cch, pwch, &bStrippedWhitespace, false /* not aggressive */ );

	// copy back, if necessary
	if ( bStrippedWhitespace )
	{
		Q_wcsncpy( pwch, pwchT, cubDest );
	}

	return bStrippedWhitespace;
}



//-----------------------------------------------------------------------------
// Purpose: strips leading and trailing whitespace,
//		and also strips punctuation and formatting characters with "clear"
//		representations.
//-----------------------------------------------------------------------------
bool Q_AggressiveStripPrecedingAndTrailingWhitespaceW( wchar_t *pwch )
{
	// duplicate on stack
	int cch = Q_wcslen( pwch );
	int cubDest = ( cch + 1 ) * sizeof( wchar_t );
	wchar_t *pwchT = (wchar_t *)stackalloc( cubDest );
	Q_wcsncpy( pwchT, pwch, cubDest );

	bool bStrippedWhitespace = false;
	pwchT = StripWhitespaceWorker( cch, pwch, &bStrippedWhitespace, true /* is aggressive */ );

	// copy back, if necessary
	if ( bStrippedWhitespace )
	{
		Q_wcsncpy( pwch, pwchT, cubDest );
	}

	return bStrippedWhitespace;
}


//-----------------------------------------------------------------------------
// Purpose: strips leading and trailing whitespace
//-----------------------------------------------------------------------------
bool Q_StripPrecedingAndTrailingWhitespace( char *pch )
{
	// convert to unicode
	int cch = Q_strlen( pch );
	int cubDest = (cch + 1 ) * sizeof( wchar_t );
	wchar_t *pwch = (wchar_t *)stackalloc( cubDest );
	int cwch = Q_UTF8ToUnicode( pch, pwch, cubDest );

	bool bStrippedWhitespace = false;
	pwch = StripWhitespaceWorker( cwch-1, pwch, &bStrippedWhitespace, false /* not aggressive */ );

	// copy back, if necessary
	if ( bStrippedWhitespace )
	{
		Q_UnicodeToUTF8( pwch, pch, cch );
	}

	return bStrippedWhitespace;
}

//-----------------------------------------------------------------------------
// Purpose: strips leading and trailing whitespace
//-----------------------------------------------------------------------------
bool Q_AggressiveStripPrecedingAndTrailingWhitespace( char *pch )
{
	// convert to unicode
	int cch = Q_strlen( pch );
	int cubDest = (cch + 1 ) * sizeof( wchar_t );
	wchar_t *pwch = (wchar_t *)stackalloc( cubDest );
	int cwch = Q_UTF8ToUnicode( pch, pwch, cubDest );

	bool bStrippedWhitespace = false;
	pwch = StripWhitespaceWorker( cwch-1, pwch, &bStrippedWhitespace, true /* is aggressive */ );

	// copy back, if necessary
	if ( bStrippedWhitespace )
	{
		Q_UnicodeToUTF8( pwch, pch, cch );
	}

	return bStrippedWhitespace;
}


//-----------------------------------------------------------------------------
// Purpose: Converts a UTF8 string into a unicode string
//-----------------------------------------------------------------------------
int V_UTF8ToUnicode( const char *pUTF8, wchar_t *pwchDest, int cubDestSizeInBytes )
{
	// pwchDest can be null to allow for getting the length of the string
	if ( cubDestSizeInBytes > 0 )
	{
		AssertValidWritePtr(pwchDest);
		pwchDest[0] = 0;
	}

	if ( !pUTF8 )
		return 0;

	AssertValidStringPtr(pUTF8);

#ifdef _WIN32
	int cchResult = MultiByteToWideChar( CP_UTF8, 0, pUTF8, -1, pwchDest, cubDestSizeInBytes / sizeof(wchar_t) );
#elif POSIX
	int cchResult = mbstowcs( pwchDest, pUTF8, cubDestSizeInBytes / sizeof(wchar_t) ) + 1;
#endif

	if ( cubDestSizeInBytes > 0 )
	{
		pwchDest[(cubDestSizeInBytes / sizeof(wchar_t)) - 1] = 0;
	}

	return cchResult;
}

//-----------------------------------------------------------------------------
// Purpose: Converts a unicode string into a UTF8 (standard) string
//-----------------------------------------------------------------------------
int V_UnicodeToUTF8( const wchar_t *pUnicode, char *pUTF8, int cubDestSizeInBytes )
{
	//AssertValidStringPtr(pUTF8, cubDestSizeInBytes); // no, we are sometimes pasing in NULL to fetch the length of the buffer needed.
	AssertValidReadPtr(pUnicode);

	if ( cubDestSizeInBytes > 0 )
	{
		pUTF8[0] = 0;
	}

#ifdef _WIN32
	int cchResult = WideCharToMultiByte( CP_UTF8, 0, pUnicode, -1, pUTF8, cubDestSizeInBytes, NULL, NULL );
#elif POSIX
	int cchResult = 0;
	if ( pUnicode && pUTF8 )
		cchResult = wcstombs( pUTF8, pUnicode, cubDestSizeInBytes ) + 1;
#endif

	if ( cubDestSizeInBytes > 0 )
	{
		pUTF8[cubDestSizeInBytes - 1] = 0;
	}

	return cchResult;
}

//-----------------------------------------------------------------------------
// Purpose: Converts a ANSI string into a unicode string
//-----------------------------------------------------------------------------
int V_ANSIToUnicode( const char *pANSI, wchar_t *pwchDest, int cubDestSizeInBytes )
{
	AssertValidStringPtr(pANSI);
	AssertValidWritePtr(pwchDest);

	pwchDest[0] = 0;
#ifdef _WIN32
	int cchResult = MultiByteToWideChar( CP_ACP, 0, pANSI, -1, pwchDest, cubDestSizeInBytes / sizeof(wchar_t) );
#elif _LINUX
#error "please fix this!"
	int cchResult = mbstowcs( pwchDest, pANSI, cubDestSizeInBytes / sizeof(wchar_t) );
#endif
	pwchDest[(cubDestSizeInBytes / sizeof(wchar_t)) - 1] = 0;
	return cchResult;
}



//-----------------------------------------------------------------------------
// Purpose: Returns the 4 bit nibble for a hex character
// Input  : c - 
// Output : unsigned char
//-----------------------------------------------------------------------------
unsigned char V_nibble( char c )
{
	if ( ( c >= '0' ) &&
		 ( c <= '9' ) )
	{
		 return (unsigned char)(c - '0');
	}

	if ( ( c >= 'A' ) &&
		 ( c <= 'F' ) )
	{
		 return (unsigned char)(c - 'A' + 0x0a);
	}

	if ( ( c >= 'a' ) &&
		 ( c <= 'f' ) )
	{
		 return (unsigned char)(c - 'a' + 0x0a);
	}

	return '0';
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *in - 
//			numchars - 
//			*out - 
//			maxoutputbytes - 
//-----------------------------------------------------------------------------
void V_hextobinary( char const *in, int numchars, byte *out, int maxoutputbytes )
{
	int len = V_strlen( in );
	numchars = min( len, numchars );
	// Make sure it's even
	numchars = ( numchars ) & ~0x1;

	// Must be an even # of input characters (two chars per output byte)
	Assert( numchars >= 2 );

	memset( out, 0x00, maxoutputbytes );

	byte *p;
	int i;

	p = out;
	for ( i = 0; 
		 ( i < numchars ) && ( ( p - out ) < maxoutputbytes ); 
		 i+=2, p++ )
	{
		*p = ( V_nibble( in[i] ) << 4 ) | V_nibble( in[i+1] );		
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *in - 
//			inputbytes - 
//			*out - 
//			outsize - 
//-----------------------------------------------------------------------------
void V_binarytohex( const byte *in, int inputbytes, char *out, int outsize )
{
	Assert( outsize >= 1 );
	char doublet[10];
	int i;

	out[0]=0;

	for ( i = 0; i < inputbytes; i++ )
	{
		unsigned char c = in[i];
		V_snprintf( doublet, sizeof( doublet ), "%02x", c );
		V_strncat( out, doublet, outsize, COPY_ALL_CHARACTERS );
	}
}

// Even though \ on Posix (Linux&Mac) isn't techincally a path separator we are
// now counting it as one even Posix since so many times our filepaths aren't actual
// paths but rather text strings passed in from data files, treating \ as a pathseparator
// covers the full range of cases
bool PATHSEPARATOR( char c )
{
	return c == '\\' || c == '/';
}


//-----------------------------------------------------------------------------
// Purpose: Extracts the base name of a file (no path, no extension, assumes '/' or '\' as path separator)
// Input  : *in - 
//			*out - 
//			maxlen - 
//-----------------------------------------------------------------------------
void V_FileBase( const char *in, char *out, int maxlen )
{
	Assert( maxlen >= 1 );
	Assert( in );
	Assert( out );

	if ( !in || !in[ 0 ] )
	{
		*out = 0;
		return;
	}

	int len, start, end;

	len = V_strlen( in );
	
	// scan backward for '.'
	end = len - 1;
	while ( end&& in[end] != '.' && !PATHSEPARATOR( in[end] ) )
	{
		end--;
	}
	
	if ( in[end] != '.' )		// no '.', copy to end
	{
		end = len-1;
	}
	else 
	{
		end--;					// Found ',', copy to left of '.'
	}

	// Scan backward for '/'
	start = len-1;
	while ( start >= 0 && !PATHSEPARATOR( in[start] ) )
	{
		start--;
	}

	if ( start < 0 || !PATHSEPARATOR( in[start] ) )
	{
		start = 0;
	}
	else 
	{
		start++;
	}

	// Length of new sting
	len = end - start + 1;

	int maxcopy = min( len + 1, maxlen );

	// Copy partial string
	V_strncpy( out, &in[start], maxcopy );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *ppath - 
//-----------------------------------------------------------------------------
void V_StripTrailingSlash( char *ppath )
{
	Assert( ppath );

	int len = V_strlen( ppath );
	if ( len > 0 )
	{
		if ( PATHSEPARATOR( ppath[ len - 1 ] ) )
		{
			ppath[ len - 1 ] = 0;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *in - 
//			*out - 
//			outSize - 
//-----------------------------------------------------------------------------
void V_StripExtension( const char *in, char *out, int outSize )
{
	// Find the last dot. If it's followed by a dot or a slash, then it's part of a 
	// directory specifier like ../../somedir/./blah.

	// scan backward for '.'
	int end = V_strlen( in ) - 1;
	while ( end > 0 && in[end] != '.' && !PATHSEPARATOR( in[end] ) )
	{
		--end;
	}

	if (end > 0 && !PATHSEPARATOR( in[end] ) && end < outSize)
	{
		int nChars = min( end, outSize-1 );
		if ( out != in )
		{
			memcpy( out, in, nChars );
		}
		out[nChars] = 0;
	}
	else
	{
		// nothing found
		if ( out != in )
		{
			V_strncpy( out, in, outSize );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *path - 
//			*extension - 
//			pathStringLength - 
//-----------------------------------------------------------------------------
void V_DefaultExtension( char *path, const char *extension, int pathStringLength )
{
	Assert( path );
	Assert( pathStringLength >= 1 );
	Assert( extension );
	Assert( extension[0] == '.' );

	char    *src;

	// if path doesn't have a .EXT, append extension
	// (extension should include the .)
	src = path + V_strlen(path) - 1;

	while ( !PATHSEPARATOR( *src ) && ( src > path ) )
	{
		if (*src == '.')
		{
			// it has an extension
			return;                 
		}
		src--;
	}

	// Concatenate the desired extension
	V_strncat( path, extension, pathStringLength, COPY_ALL_CHARACTERS );
}

//-----------------------------------------------------------------------------
// Purpose: Force extension...
// Input  : *path - 
//			*extension - 
//			pathStringLength - 
//-----------------------------------------------------------------------------
void V_SetExtension( char *path, const char *extension, int pathStringLength )
{
	V_StripExtension( path, path, pathStringLength );
	V_DefaultExtension( path, extension, pathStringLength );
}

//-----------------------------------------------------------------------------
// Purpose: Remove final filename from string
// Input  : *path - 
// Output : void  V_StripFilename
//-----------------------------------------------------------------------------
void  V_StripFilename (char *path)
{
	int             length;

	length = V_strlen( path )-1;
	if ( length <= 0 )
		return;

	while ( length > 0 && 
		!PATHSEPARATOR( path[length] ) )
	{
		length--;
	}

	path[ length ] = 0;
}

#ifdef _WIN32
#define CORRECT_PATH_SEPARATOR '\\'
#define INCORRECT_PATH_SEPARATOR '/'
#elif POSIX
#define CORRECT_PATH_SEPARATOR '/'
#define INCORRECT_PATH_SEPARATOR '\\'
#endif

//-----------------------------------------------------------------------------
// Purpose: Changes all '/' or '\' characters into separator
// Input  : *pname - 
//			separator - 
//-----------------------------------------------------------------------------
void V_FixSlashes( char *pname, char separator /* = CORRECT_PATH_SEPARATOR */ )
{
	while ( *pname )
	{
		if ( *pname == INCORRECT_PATH_SEPARATOR || *pname == CORRECT_PATH_SEPARATOR )
		{
			*pname = separator;
		}
		pname++;
	}
}


//-----------------------------------------------------------------------------
// Purpose: This function fixes cases of filenames like materials\\blah.vmt or somepath\otherpath\\ and removes the extra double slash.
//-----------------------------------------------------------------------------
void V_FixDoubleSlashes( char *pStr )
{
	int len = V_strlen( pStr );

	for ( int i=1; i < len-1; i++ )
	{
		if ( (pStr[i] == '/' || pStr[i] == '\\') && (pStr[i+1] == '/' || pStr[i+1] == '\\') )
		{
			// This means there's a double slash somewhere past the start of the filename. That 
			// can happen in Hammer if they use a material in the root directory. You'll get a filename 
			// that looks like 'materials\\blah.vmt'
			V_memmove( &pStr[i], &pStr[i+1], len - i );
			--len;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Strip off the last directory from dirName
// Input  : *dirName - 
//			maxlen - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool V_StripLastDir( char *dirName, int maxlen )
{
	if( dirName[0] == 0 || 
		!V_stricmp( dirName, "./" ) || 
		!V_stricmp( dirName, ".\\" ) )
		return false;
	
	int len = V_strlen( dirName );

	Assert( len < maxlen );

	// skip trailing slash
	if ( PATHSEPARATOR( dirName[len-1] ) )
	{
		len--;
	}

	while ( len > 0 )
	{
		if ( PATHSEPARATOR( dirName[len-1] ) )
		{
			dirName[len] = 0;
			V_FixSlashes( dirName, CORRECT_PATH_SEPARATOR );
			return true;
		}
		len--;
	}

	// Allow it to return an empty string and true. This can happen if something like "tf2/" is passed in.
	// The correct behavior is to strip off the last directory ("tf2") and return true.
	if( len == 0 )
	{
		V_snprintf( dirName, maxlen, ".%c", CORRECT_PATH_SEPARATOR );
		return true;
	}

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: Returns a pointer to the beginning of the unqualified file name 
//			(no path information)
// Input:	in - file name (may be unqualified, relative or absolute path)
// Output:	pointer to unqualified file name
//-----------------------------------------------------------------------------
const char * V_UnqualifiedFileName( const char * in )
{
	// back up until the character after the first path separator we find,
	// or the beginning of the string
	const char * out = in + strlen( in ) - 1;
	while ( ( out > in ) && ( !PATHSEPARATOR( *( out-1 ) ) ) )
		out--;
	return out;
}


//-----------------------------------------------------------------------------
// Purpose: Composes a path and filename together, inserting a path separator
//			if need be
// Input:	path - path to use
//			filename - filename to use
//			dest - buffer to compose result in
//			destSize - size of destination buffer
//-----------------------------------------------------------------------------
void V_ComposeFileName( const char *path, const char *filename, char *dest, int destSize )
{
	V_strncpy( dest, path, destSize );
	V_FixSlashes( dest );
	V_AppendSlash( dest, destSize );
	V_strncat( dest, filename, destSize, COPY_ALL_CHARACTERS );
	V_FixSlashes( dest );
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *path - 
//			*dest - 
//			destSize - 
// Output : void V_ExtractFilePath
//-----------------------------------------------------------------------------
bool V_ExtractFilePath (const char *path, char *dest, int destSize )
{
	Assert( destSize >= 1 );
	if ( destSize < 1 )
	{
		return false;
	}

	// Last char
	int len = V_strlen(path);
	const char *src = path + (len ? len-1 : 0);

	// back up until a \ or the start
	while ( src != path && !PATHSEPARATOR( *(src-1) ) )
	{
		src--;
	}

	int copysize = min( src - path, destSize - 1 );
	memcpy( dest, path, copysize );
	dest[copysize] = 0;

	return copysize != 0 ? true : false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *path - 
//			*dest - 
//			destSize - 
// Output : void V_ExtractFileExtension
//-----------------------------------------------------------------------------
void V_ExtractFileExtension( const char *path, char *dest, int destSize )
{
	*dest = NULL;
	const char * extension = V_GetFileExtension( path );
	if ( NULL != extension )
		V_strncpy( dest, extension, destSize );
}


//-----------------------------------------------------------------------------
// Purpose: Returns a pointer to the file extension within a file name string
// Input:	in - file name 
// Output:	pointer to beginning of extension (after the "."), or NULL
//				if there is no extension
//-----------------------------------------------------------------------------
const char * V_GetFileExtension( const char * path )
{
	const char    *src;

	src = path + strlen(path) - 1;

//
// back up until a . or the start
//
	while (src != path && *(src-1) != '.' )
		src--;

	// check to see if the '.' is part of a pathname
	if (src == path || PATHSEPARATOR( *src ) )
	{		
		return NULL;  // no extension
	}

	return src;
}


//-----------------------------------------------------------------------------
// Purpose: Returns a pointer to the filename part of a path string
// Input:	in - file name 
// Output:	pointer to beginning of filename (after the "/"). If there were no /, 
//          output is identical to input
//-----------------------------------------------------------------------------
const char * V_GetFileName( const char * path )
{
	return V_UnqualifiedFileName( path );
}


bool V_RemoveDotSlashes( char *pFilename, char separator, bool bRemoveDoubleSlashes /* = true */ )
{
	char *pIn = pFilename;
	char *pOut = pFilename;
	bool bRetVal = true;

	bool bBoundary = true;
	while ( *pIn )
	{
		if ( bBoundary && pIn[0] == '.' && pIn[1] == '.' && ( PATHSEPARATOR( pIn[2] ) || !pIn[2] ) )
		{
			// Get rid of /../ or trailing /.. by backing pOut up to previous separator

			// Eat the last separator (or repeated separators) we wrote out
			while ( pOut != pFilename && pOut[-1] == separator )
			{
				--pOut;
			}

			while ( true )
			{
				if ( pOut == pFilename )
				{
					bRetVal = false; // backwards compat. return value, even though we continue handling
					break;
				}
				--pOut;
				if ( *pOut == separator )
				{
					break;
				}
			}

			// Skip the '..' but not the slash, next loop iteration will handle separator
			pIn += 2;
			bBoundary = ( pOut == pFilename );
		}
		else if ( bBoundary && pIn[0] == '.' && ( PATHSEPARATOR( pIn[1] ) || !pIn[1] ) )
		{
			// Handle "./" by simply skipping this sequence. bBoundary is unchanged.
			if ( PATHSEPARATOR( pIn[1] ) )
			{
				pIn += 2;
			}
			else
			{
				// Special case: if trailing "." is preceded by separator, eg "path/.",
				// then the final separator should also be stripped. bBoundary may then
				// be in an incorrect state, but we are at the end of processing anyway
				// so we don't really care (the processing loop is about to terminate).
				if ( pOut != pFilename && pOut[-1] == separator )
				{
					--pOut;
				}
				pIn += 1;
			}
		}
		else if ( PATHSEPARATOR( pIn[0] ) )
		{
			*pOut = separator;
			pOut += 1 - (bBoundary & bRemoveDoubleSlashes & (pOut != pFilename));
			pIn += 1;
			bBoundary = true;
		}
		else
		{
			if ( pOut != pIn )
			{
				*pOut = *pIn;
			}
			pOut += 1;
			pIn += 1;
			bBoundary = false;
		}
	}
	*pOut = 0;

	return bRetVal;
}


void V_AppendSlash( char *pStr, int strSize )
{
	int len = V_strlen( pStr );
	if ( len > 0 && !PATHSEPARATOR(pStr[len-1]) )
	{
		if ( len+1 >= strSize )
			Error( "V_AppendSlash: ran out of space on %s.", pStr );
		
		pStr[len] = CORRECT_PATH_SEPARATOR;
		pStr[len+1] = 0;
	}
}


void V_MakeAbsolutePath( char *pOut, int outLen, const char *pPath, const char *pStartingDir )
{
	if ( V_IsAbsolutePath( pPath ) )
	{
		// pPath is not relative.. just copy it.
		V_strncpy( pOut, pPath, outLen );
	}
	else
	{
		// Make sure the starting directory is absolute..
		if ( pStartingDir && V_IsAbsolutePath( pStartingDir ) )
		{
			V_strncpy( pOut, pStartingDir, outLen );
		}
		else
		{
			if ( !_getcwd( pOut, outLen ) )
				Error( "V_MakeAbsolutePath: _getcwd failed." );

			if ( pStartingDir )
			{
				V_AppendSlash( pOut, outLen );
				V_strncat( pOut, pStartingDir, outLen, COPY_ALL_CHARACTERS );
			}
		}

		// Concatenate the paths.
		V_AppendSlash( pOut, outLen );
		V_strncat( pOut, pPath, outLen, COPY_ALL_CHARACTERS );
	}

	if ( !V_RemoveDotSlashes( pOut ) )
		Error( "V_MakeAbsolutePath: tried to \"..\" past the root." );

	//V_FixSlashes( pOut ); - handled by V_RemoveDotSlashes
}


//-----------------------------------------------------------------------------
// Makes a relative path
//-----------------------------------------------------------------------------
bool V_MakeRelativePath( const char *pFullPath, const char *pDirectory, char *pRelativePath, int nBufLen )
{
	pRelativePath[0] = 0;

	const char *pPath = pFullPath;
	const char *pDir = pDirectory;

	// Strip out common parts of the path
	const char *pLastCommonPath = NULL;
	const char *pLastCommonDir = NULL;
	while ( *pPath && ( FastToLower( *pPath ) == FastToLower( *pDir ) || 
						( PATHSEPARATOR( *pPath ) && ( PATHSEPARATOR( *pDir ) || (*pDir == 0) ) ) ) )
	{
		if ( PATHSEPARATOR( *pPath ) )
		{
			pLastCommonPath = pPath + 1;
			pLastCommonDir = pDir + 1;
		}
		if ( *pDir == 0 )
		{
			--pLastCommonDir;
			break;
		}
		++pDir; ++pPath;
	}

	// Nothing in common
	if ( !pLastCommonPath )
		return false;

	// For each path separator remaining in the dir, need a ../
	int nOutLen = 0;
	bool bLastCharWasSeparator = true;
	for ( ; *pLastCommonDir; ++pLastCommonDir )
	{
		if ( PATHSEPARATOR( *pLastCommonDir ) )
		{
			pRelativePath[nOutLen++] = '.';
			pRelativePath[nOutLen++] = '.';
			pRelativePath[nOutLen++] = CORRECT_PATH_SEPARATOR;
			bLastCharWasSeparator = true;
		}
		else
		{
			bLastCharWasSeparator = false;
		}
	}

	// Deal with relative paths not specified with a trailing slash
	if ( !bLastCharWasSeparator )
	{
		pRelativePath[nOutLen++] = '.';
		pRelativePath[nOutLen++] = '.';
		pRelativePath[nOutLen++] = CORRECT_PATH_SEPARATOR;
	}

	// Copy the remaining part of the relative path over, fixing the path separators
	for ( ; *pLastCommonPath; ++pLastCommonPath )
	{
		if ( PATHSEPARATOR( *pLastCommonPath ) )
		{
			pRelativePath[nOutLen++] = CORRECT_PATH_SEPARATOR;
		}
		else
		{
			pRelativePath[nOutLen++] = *pLastCommonPath;
		}

		// Check for overflow
		if ( nOutLen == nBufLen - 1 )
			break;
	}

	pRelativePath[nOutLen] = 0;
	return true;
}


//-----------------------------------------------------------------------------
// small helper function shared by lots of modules
//-----------------------------------------------------------------------------
bool V_IsAbsolutePath( const char *pStr )
{
	bool bIsAbsolute = ( pStr[0] && pStr[1] == ':' ) || pStr[0] == '/' || pStr[0] == '\\';
	if ( IsX360() && !bIsAbsolute )
	{
		bIsAbsolute = ( V_stristr( pStr, ":" ) != NULL );
	}
	return bIsAbsolute;
}


// Copies at most nCharsToCopy bytes from pIn into pOut.
// Returns false if it would have overflowed pOut's buffer.
static bool CopyToMaxChars( char *pOut, int outSize, const char *pIn, int nCharsToCopy )
{
	if ( outSize == 0 )
		return false;

	int iOut = 0;
	while ( *pIn && nCharsToCopy > 0 )
	{
		if ( iOut == (outSize-1) )
		{
			pOut[iOut] = 0;
			return false;
		}
		pOut[iOut] = *pIn;
		++iOut;
		++pIn;
		--nCharsToCopy;
	}
	
	pOut[iOut] = 0;
	return true;
}


//-----------------------------------------------------------------------------
// Fixes up a file name, removing dot slashes, fixing slashes, converting to lowercase, etc.
//-----------------------------------------------------------------------------
void V_FixupPathName( char *pOut, size_t nOutLen, const char *pPath )
{
	V_strncpy( pOut, pPath, nOutLen );
	V_RemoveDotSlashes( pOut, CORRECT_PATH_SEPARATOR, true );
#ifdef WIN32
	V_strlower( pOut );
#endif
}


// Returns true if it completed successfully.
// If it would overflow pOut, it fills as much as it can and returns false.
bool V_StrSubst( 
	const char *pIn, 
	const char *pMatch,
	const char *pReplaceWith,
	char *pOut,
	int outLen,
	bool bCaseSensitive
	)
{
	int replaceFromLen = strlen( pMatch );
	int replaceToLen = strlen( pReplaceWith );

	const char *pInStart = pIn;
	char *pOutPos = pOut;
	pOutPos[0] = 0;

	while ( 1 )
	{
		int nRemainingOut = outLen - (pOutPos - pOut);

		const char *pTestPos = ( bCaseSensitive ? strstr( pInStart, pMatch ) : V_stristr( pInStart, pMatch ) );
		if ( pTestPos )
		{
			// Found an occurence of pMatch. First, copy whatever leads up to the string.
			int copyLen = pTestPos - pInStart;
			if ( !CopyToMaxChars( pOutPos, nRemainingOut, pInStart, copyLen ) )
				return false;
			
			// Did we hit the end of the output string?
			if ( copyLen > nRemainingOut-1 )
				return false;

			pOutPos += strlen( pOutPos );
			nRemainingOut = outLen - (pOutPos - pOut);

			// Now add the replacement string.
			if ( !CopyToMaxChars( pOutPos, nRemainingOut, pReplaceWith, replaceToLen ) )
				return false;

			pInStart += copyLen + replaceFromLen;
			pOutPos += replaceToLen;			
		}
		else
		{
			// We're at the end of pIn. Copy whatever remains and get out.
			int copyLen = strlen( pInStart );
			V_strncpy( pOutPos, pInStart, nRemainingOut );
			return ( copyLen <= nRemainingOut-1 );
		}
	}
}


char* AllocString( const char *pStr, int nMaxChars )
{
	int allocLen;
	if ( nMaxChars == -1 )
		allocLen = strlen( pStr ) + 1;
	else
		allocLen = min( (int)strlen(pStr), nMaxChars ) + 1;

	char *pOut = new char[allocLen];
	V_strncpy( pOut, pStr, allocLen );
	return pOut;
}


void V_SplitString2( const char *pString, const char **pSeparators, int nSeparators, CUtlVector<char*> &outStrings )
{
	outStrings.Purge();
	const char *pCurPos = pString;
	while ( 1 )
	{
		int iFirstSeparator = -1;
		const char *pFirstSeparator = 0;
		for ( int i=0; i < nSeparators; i++ )
		{
			const char *pTest = V_stristr( pCurPos, pSeparators[i] );
			if ( pTest && (!pFirstSeparator || pTest < pFirstSeparator) )
			{
				iFirstSeparator = i;
				pFirstSeparator = pTest;
			}
		}

		if ( pFirstSeparator )
		{
			// Split on this separator and continue on.
			int separatorLen = strlen( pSeparators[iFirstSeparator] );
			if ( pFirstSeparator > pCurPos )
			{
				outStrings.AddToTail( AllocString( pCurPos, pFirstSeparator-pCurPos ) );
			}

			pCurPos = pFirstSeparator + separatorLen;
		}
		else
		{
			// Copy the rest of the string
			if ( strlen( pCurPos ) )
			{
				outStrings.AddToTail( AllocString( pCurPos, -1 ) );
			}
			return;
		}
	}
}


void V_SplitString( const char *pString, const char *pSeparator, CUtlVector<char*> &outStrings )
{
	V_SplitString2( pString, &pSeparator, 1, outStrings );
}


bool V_GetCurrentDirectory( char *pOut, int maxLen )
{
	return _getcwd( pOut, maxLen ) == pOut;
}


bool V_SetCurrentDirectory( const char *pDirName )
{
	return _chdir( pDirName ) == 0;
}


// This function takes a slice out of pStr and stores it in pOut.
// It follows the Python slice convention:
// Negative numbers wrap around the string (-1 references the last character).
// Numbers are clamped to the end of the string.
void V_StrSlice( const char *pStr, int firstChar, int lastCharNonInclusive, char *pOut, int outSize )
{
	if ( outSize == 0 )
		return;
	
	int length = strlen( pStr );

	// Fixup the string indices.
	if ( firstChar < 0 )
	{
		firstChar = length - (-firstChar % length);
	}
	else if ( firstChar >= length )
	{
		pOut[0] = 0;
		return;
	}

	if ( lastCharNonInclusive < 0 )
	{
		lastCharNonInclusive = length - (-lastCharNonInclusive % length);
	}
	else if ( lastCharNonInclusive > length )
	{
		lastCharNonInclusive %= length;
	}

	if ( lastCharNonInclusive <= firstChar )
	{
		pOut[0] = 0;
		return;
	}

	int copyLen = lastCharNonInclusive - firstChar;
	if ( copyLen <= (outSize-1) )
	{
		memcpy( pOut, &pStr[firstChar], copyLen );
		pOut[copyLen] = 0;
	}
	else
	{
		memcpy( pOut, &pStr[firstChar], outSize-1 );
		pOut[outSize-1] = 0;
	}
}


void V_StrLeft( const char *pStr, int nChars, char *pOut, int outSize )
{
	if ( nChars == 0 )
	{
		if ( outSize != 0 )
			pOut[0] = 0;

		return;
	}

	V_StrSlice( pStr, 0, nChars, pOut, outSize );
}


void V_StrRight( const char *pStr, int nChars, char *pOut, int outSize )
{
	int len = strlen( pStr );
	if ( nChars >= len )
	{
		V_strncpy( pOut, pStr, outSize );
	}
	else
	{
		V_StrSlice( pStr, -nChars, strlen( pStr ), pOut, outSize );
	}
}

//-----------------------------------------------------------------------------
// Convert multibyte to wchar + back
//-----------------------------------------------------------------------------
void V_strtowcs( const char *pString, int nInSize, wchar_t *pWString, int nOutSizeInBytes )
{
	Assert( nOutSizeInBytes >= sizeof(pWString[0]) );
#ifdef _WIN32
	int nOutSizeInChars = nOutSizeInBytes / sizeof(pWString[0]);
	int result = MultiByteToWideChar( CP_UTF8, 0, pString, nInSize, pWString, nOutSizeInChars );
	// If the string completely fails to fit then MultiByteToWideChar will return 0.
	// If the string exactly fits but with no room for a null-terminator then MultiByteToWideChar
	// will happily fill the buffer and omit the null-terminator, returning nOutSizeInChars.
	// Either way we need to return an empty string rather than a bogus and possibly not
	// null-terminated result.
	if ( result <= 0 || result >= nOutSizeInChars )
	{
		// If nInSize includes the null-terminator then a result of nOutSizeInChars is
		// legal. We check this by seeing if the last character in the output buffer is
		// a zero.
		if ( result == nOutSizeInChars && pWString[ nOutSizeInChars - 1 ] == 0)
		{
			// We're okay! Do nothing.
		}
		else
		{
			// The string completely to fit. Null-terminate the buffer.
			*pWString = L'\0';
		}
	}
	else
	{
		// We have successfully converted our string. Now we need to null-terminate it, because
		// MultiByteToWideChar will only do that if nInSize includes the source null-terminator!
		pWString[ result ] = 0;
	}
#elif POSIX
	if ( mbstowcs( pWString, pString, nOutSizeInBytes / sizeof(pWString[0]) ) <= 0 )
	{
		*pWString = 0;
	}
#endif
}

void V_wcstostr( const wchar_t *pWString, int nInSize, char *pString, int nOutSizeInChars )
{
#ifdef _WIN32
	int result = WideCharToMultiByte( CP_UTF8, 0, pWString, nInSize, pString, nOutSizeInChars, NULL, NULL );
	// If the string completely fails to fit then MultiByteToWideChar will return 0.
	// If the string exactly fits but with no room for a null-terminator then MultiByteToWideChar
	// will happily fill the buffer and omit the null-terminator, returning nOutSizeInChars.
	// Either way we need to return an empty string rather than a bogus and possibly not
	// null-terminated result.
	if ( result <= 0 || result >= nOutSizeInChars )
	{
		// If nInSize includes the null-terminator then a result of nOutSizeInChars is
		// legal. We check this by seeing if the last character in the output buffer is
		// a zero.
		if ( result == nOutSizeInChars && pWString[ nOutSizeInChars - 1 ] == 0)
		{
			// We're okay! Do nothing.
		}
		else
		{
			*pString = '\0';
		}
	}
	else
	{
		// We have successfully converted our string. Now we need to null-terminate it, because
		// MultiByteToWideChar will only do that if nInSize includes the source null-terminator!
		pString[ result ] = '\0';
	}
#elif POSIX
	if ( wcstombs( pString, pWString, nOutSizeInChars ) <= 0 )
	{
		*pString = '\0';
	}
#endif
}



//--------------------------------------------------------------------------------
// backslashification
//--------------------------------------------------------------------------------

static char s_BackSlashMap[]="\tt\nn\rr\"\"\\\\";

char *V_AddBackSlashesToSpecialChars( char const *pSrc )
{
	// first, count how much space we are going to need
	int nSpaceNeeded = 0;
	for( char const *pScan = pSrc; *pScan; pScan++ )
	{
		nSpaceNeeded++;
		for(char const *pCharSet=s_BackSlashMap; *pCharSet; pCharSet += 2 )
		{
			if ( *pCharSet == *pScan )
				nSpaceNeeded++;								// we need to store a bakslash
		}
	}
	char *pRet = new char[ nSpaceNeeded + 1 ];				// +1 for null
	char *pOut = pRet;
	
	for( char const *pScan = pSrc; *pScan; pScan++ )
	{
		bool bIsSpecial = false;
		for(char const *pCharSet=s_BackSlashMap; *pCharSet; pCharSet += 2 )
		{
			if ( *pCharSet == *pScan )
			{
				*( pOut++ ) = '\\';
				*( pOut++ ) = pCharSet[1];
				bIsSpecial = true;
				break;
			}
		}
		if (! bIsSpecial )
		{
			*( pOut++ ) = *pScan;
		}
	}
	*( pOut++ ) = 0;
	return pRet;
}
#if defined( LINUX ) || defined( _PS3 )
extern "C" void qsort_s( void *base, size_t num, size_t width, int (*compare )(void *, const void *, const void *), void * context );
#endif

void V_qsort_s( void *base, size_t num, size_t width, int ( __cdecl *compare )(void *, const void *, const void *), void * context ) 
{
#if defined OSX
	// the arguments are swapped 'round on the mac - awesome, huh?
	return qsort_r( base, num, width, context, compare );
#else
	return qsort_s( base, num, width, compare, context );
#endif
}
