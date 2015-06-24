typedef enum
{
	SPR_SINGLE,
	SPR_GROUP,
}
spriteframetype_t;

typedef struct mspriteframe_s
{
	int width;
	int height;
	float up;
	float down;
	float left;
	float right;
	int gl_texturenum;
}
mspriteframe_t;

typedef struct
{
	spriteframetype_t type;
	mspriteframe_t *frameptr;
}
mspriteframedesc_t;

typedef struct msprite_s
{
	short type;
	short texFormat;
	int maxwidth;
	int maxheight;
	int numframes;
	int paloffset;
	float beamlength;
	void *cachespot;
	mspriteframedesc_t frames[1];
}
msprite_t;