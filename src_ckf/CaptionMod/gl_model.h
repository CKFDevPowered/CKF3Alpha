#pragma once

typedef struct
{
	float texcoord[4][2];
}tgasprite_frame_t;

typedef struct
{
	bool isValid;
	tgasprite_frame_t *frame;
	int numframes;
	int w, h;
	int tex;
}tgasprite_t;

enum
{
	kRenderAddColor = 6,
	kRenderTransAlphaNoDepth,
	kRenderShaderConc
};

#define SPR_VP_PARALLEL_UPRIGHT 0
#define SPR_FACING_UPRIGHT 1
#define SPR_VP_PARALLEL 2
#define SPR_ORIENTED 3
#define SPR_VP_PARALLEL_ORIENTED 4

#define SPR_NORMAL 0
#define SPR_ADDITIVE 1
#define SPR_INDEXALPHA 2
#define SPR_ALPHTEST 3