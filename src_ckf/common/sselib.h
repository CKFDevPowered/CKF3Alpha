#pragma once

//SSE optimization

#define shuffle(a, b, c) (((a)<<4) | ((b)<<2) | ((c)))

//==========================
// SSE DotProduct Plane EQ
//
//==========================
inline void SSEDotProductSub(float *result, vec3_t *v0, vec3_t *v1, float *subval )
{
	_asm{
		mov             esi,    v0
		mov             edi,    v1
		mov             eax,    result;
		mov             edx,    subval;

		movups  xmm0,   [esi];
		movups  xmm1,   [edi];

		mulps   xmm0,   xmm1;

		movups  xmm2,   xmm0;
		shufps  xmm2,   xmm0, shuffle(0x01, 0x00, 0x02);
		addps   xmm2,   xmm0;
		shufps  xmm0,   xmm2, shuffle(0x02, 0x00, 0x01);
		addps   xmm0,   xmm2;

		subss   xmm0,   [edx];
		movss   [eax],  xmm0;
	}
}
 
//==========================
// SSE DotProduct world coord on Studio Models
//
//==========================
inline void SSEDotProductWorld( float* result, const float* v0, const float* v1 )
{
	_asm{
		mov             esi,    v0
		mov             edi,    v1
		mov             eax,    result;

		movups  xmm0,   [esi];
		movups  xmm1,   [edi];

		mulps   xmm0,   xmm1;

		movups  xmm2,   xmm0;
		shufps  xmm2,   xmm0, shuffle(0x01, 0x00, 0x02);
		addps   xmm2,   xmm0;
		shufps  xmm0,   xmm2, shuffle(0x02, 0x00, 0x01);
		addps   xmm0,   xmm2;

		addss   xmm0,   [edi+12];
		movss   [eax],  xmm0;
	}
}
 
//==========================
// SSE DotProduct
//
//==========================
inline void DotProductSSE( float* result, const float* v0, const float* v1 )
{
	_asm{
		mov             esi,    v0
		mov             edi,    v1
		mov             eax,    result;

		movups  xmm0,   [esi];
		movups  xmm1,   [edi];

		mulps   xmm0,   xmm1;

		movups  xmm2,   xmm0;
		shufps  xmm2,   xmm0, shuffle(0x01, 0x00, 0x02);
		addps   xmm2,   xmm0;
		shufps  xmm0,   xmm2, shuffle(0x02, 0x00, 0x01);
		addps   xmm0,   xmm2;

		movss   [eax],  xmm0;   
	}
}

/*
====================
VectorAddSSE

====================
*/
inline void VectorAddSSE( const float* v0, const float* v1, const float* result )
{
	_asm {
		mov	esi,	v0
		mov	edi,	v1
		mov	eax,	result

		movss	xmm0,	[esi]
		movss	xmm1,	[esi+4]
		movss   xmm2,	[esi+8]

		addss	xmm0,   [edi]
		addss	xmm1,   [edi+4]
		addss	xmm2,   [edi+8]

		movss	[eax],		xmm0
		movss	[eax+4],	xmm1
		movss	[eax+8],	xmm2
	}
}

/*
====================
VectorSubtract

====================
*/
inline void VectorMASSE (const float *veca, float scale, const float *vecb, float *vecc)
{
	_asm {
		mov		eax,  veca;
		mov		ebx,  vecb;
		mov		ecx,  vecc;
		movss	xmm7, scale;

		;scale*vecb
		movss	xmm0, [ebx];
		movss	xmm1, [ebx+4];
		movss	xmm2, [ebx+8];
		mulss	xmm0, xmm7;
		mulss	xmm1, xmm7;
		mulss	xmm2, xmm7;

		;(scale*vecb) + veca
		movss	xmm3, [eax];
		movss	xmm4, [eax+4];
		movss	xmm5, [eax+8];
		addss	xmm0, xmm3;
		addss	xmm1, xmm4;
		addss	xmm2, xmm5;

		;return_it
		movss	[ecx],   xmm0;
		movss	[ecx+4], xmm1;
		movss	[ecx+8], xmm2;
	}
}

/*
====================
VectorRotateSSE

====================
*/
inline void VectorRotateSSE (const float *in1, float in2[3][4], float *out)
{
	DotProductSSE(&out[0], in1, in2[0]);
	DotProductSSE(&out[1], in1, in2[1]);
	DotProductSSE(&out[2], in1, in2[2]);
}

/*
====================
VectorTransformSSE

====================
*/
inline void VectorTransformSSE(const float *in1, float in2[3][4], float *out)
{
	SSEDotProductWorld(&out[0], in1, in2[0]);
	SSEDotProductWorld(&out[1], in1, in2[1]);
	SSEDotProductWorld(&out[2], in1, in2[2]);
}

inline void VectorScaleSSE(float *v, float scale)
{
	_asm {
		mov		eax,  v;
		movss	xmm7, scale;

		;scale*vecb
		movss	xmm0, [eax];
		movss	xmm1, [eax+4];
		movss	xmm2, [eax+8];
		mulss	xmm0, xmm7;
		mulss	xmm1, xmm7;
		mulss	xmm2, xmm7;

		;return_it
		movss	[eax],   xmm0;
		movss	[eax+4], xmm1;
		movss	[eax+8], xmm2;
	}
}

inline void sqrtSSE(float *result, float val)
{
	_asm {
		mov		eax,  result;
		movss	xmm0, val;
		sqrtss	xmm0, xmm0;
		movss	[eax], xmm0;
	}
}

inline float VectorLengthSSE(float *v)
{
	float sqroot, length;
	DotProductSSE(&length, v, v);
	sqrtSSE(&sqroot, length);
	return sqroot;
}

inline void rsqrtSSE(float *result, float val)
{
	_asm {
		mov		eax,  result;
		movss	xmm0, val;
		rsqrtss	xmm0, xmm0;
		movss	[eax], xmm0;
	}
}

inline void VectorNormalizeSSE(float *v)
{
	float rsqroot, length;
	DotProductSSE(&length, v, v);
	rsqrtSSE(&rsqroot, length);
	VectorScaleSSE(v, rsqroot);
}