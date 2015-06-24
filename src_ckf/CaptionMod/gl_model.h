#pragma once

typedef struct
{
	float texcoord[4][2];
}tgasprite_frame_t;

typedef struct
{
	tgasprite_frame_t *frame;
	int numframes;
	int w, h;
	int tex;
}tgasprite_t;

enum
{
	kRenderAddColor = 6,
	kRenderTransAlphaNoDepth
};