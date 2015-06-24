#define MAX_GLTEXTURES 4800

typedef struct
{
	int texnum;
	int servercount;
	int paletteIndex;
	int width;
	int height;
	qboolean mipmap;
	char identifier[64];
}
gltexture_t;

typedef enum
{
	GLT_SYSTEM,
	GLT_DECAL,
	GLT_HUDSPRITE,
	GLT_STUDIO,
	GLT_WORLD,
	GLT_SPRITE
}
GL_TEXTURETYPE;

#define TEX_TYPE_NONE 0
#define TEX_TYPE_ALPHA 1
#define TEX_TYPE_LUM 2
#define TEX_TYPE_ALPHA_GRADIENT 3
#define TEX_TYPE_RGBA 4

#define TEX_IS_ALPHA(type) ((type) == TEX_TYPE_ALPHA || (type) == TEX_TYPE_ALPHA_GRADIENT || (type) == TEX_TYPE_RGBA)

int GL_LoadTexture(char *identifier, GL_TEXTURETYPE textureType, int width, int height, byte *data, qboolean mipmap, int iType, byte *pPal);
int GL_LoadTexture2(char *identifier, GL_TEXTURETYPE textureType, int width, int height, byte *data, qboolean mipmap, int iType, byte *pPal, int filter);