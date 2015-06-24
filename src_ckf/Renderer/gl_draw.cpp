#include "gl_local.h"
#include "command.h"

//For PNG Support
extern "C"
{
#include "libpng/png.h"
#include "libpng/pnginfo.h"
};

GLenum TEXTURE0_SGIS;
GLenum TEXTURE1_SGIS;
GLenum TEXTURE2_SGIS;
GLenum TEXTURE3_SGIS;

xcommand_t gl_texturemode_function;

float current_ansio = -1.0;

static byte texloader_buffer[2048*2048*4];
gltexture_t *gltextures;
int *numgltextures;
int *gHostSpawnCount;
int *currenttexid;//for 3xxx~4xxx
int *currenttexture;
//for renderer
gltexture_t *currentglt;
static byte scaled_buffer[1024*1024*4];
int gl_filter_min = GL_LINEAR_MIPMAP_LINEAR;
int gl_filter_max = GL_LINEAR;

int gl_loadtexture_format;
int gl_loadtexture_size;
//For PNG Support
#pragma comment(lib,"zlib.lib")
#pragma comment(lib,"libpng.lib")

GLuint GL_GenTexture(void)
{
	GLuint tex;
	if(g_dwEngineBuildnum < 5953)
	{
		tex = (*currenttexid);
		(*currenttexid) ++;
	}
	else
	{
		qglGenTextures(1, &tex);
	}
	return tex;
}

void GL_DeleteTexture(GLuint tex)
{
	qglDeleteTextures(1, &tex);
}

void GL_Bind(int texnum)
{
	gRefFuncs.GL_Bind(texnum);
}

void GL_SelectTexture(GLenum target)
{
	gRefFuncs.GL_SelectTexture(target);
}

void GL_DisableMultitexture(void)
{
	gRefFuncs.GL_DisableMultitexture();
}

void GL_EnableMultitexture(void)
{
	gRefFuncs.GL_EnableMultitexture();
}

int GL_LoadTexture2(char *identifier, GL_TEXTURETYPE textureType, int width, int height, byte *data, qboolean mipmap, int iType, byte *pPal, int filter)
{
	if (!mipmap || textureType != GLT_WORLD)
		return gRefFuncs.GL_LoadTexture2(identifier, textureType, width, height, data, mipmap, iType, pPal, filter);

	Draw_UpdateAnsios();

	if(gl_force_ansio)
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, gl_force_ansio);
	else
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max(min(gl_ansio->value, gl_max_ansio), 1));

	return gRefFuncs.GL_LoadTexture2(identifier, textureType, width, height, data, mipmap, iType, pPal, filter);
}

typedef struct
{
	char *name;
	int minimize, maximize;
}glmode_t;

glmode_t modes[] =
{
	{ "GL_NEAREST", GL_NEAREST, GL_NEAREST },
	{ "GL_LINEAR", GL_LINEAR, GL_LINEAR },
	{ "GL_NEAREST_MIPMAP_NEAREST", GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST },
	{ "GL_LINEAR_MIPMAP_NEAREST", GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR },
	{ "GL_NEAREST_MIPMAP_LINEAR", GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST },
	{ "GL_LINEAR_MIPMAP_LINEAR", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR }
};

void Draw_TextureMode_f(void)
{
	int i;

	if (gEngfuncs.Cmd_Argc() == 1)
	{
		for (i = 0; i < 6; i++)
		{
			if (gl_filter_min == modes[i].minimize)
			{
				gEngfuncs.Con_Printf("%s\n", modes[i].name);
				return;
			}
		}

		gEngfuncs.Con_Printf("current filter is unknown???\n");
		return;
	}

	for (i = 0; i < 6; i++)
	{
		if (!stricmp(modes[i].name, gEngfuncs.Cmd_Argv(1)))
			break;
	}

	if (i == 6)
	{
		gEngfuncs.Con_Printf("bad filter name\n");
		return;
	}

	gl_filter_min = modes[i].minimize;
	gl_filter_max = modes[i].maximize;
}

void Draw_UpdateAnsios(void)
{
	int i;

	if (gl_ansio->value != current_ansio)
	{
		for (i = 0; i < 6; i++)
		{
			if (gl_filter_min == modes[i].minimize)
			{
				char cmd[64];
				sprintf(cmd, "gl_texturemode %s\n", modes[i].name);
				gEngfuncs.pfnClientCmd(cmd);
				break;
			}
		}

		current_ansio = gl_ansio->value;
	}
}

void Draw_Init(void)
{
	gl_texturemode_function = Cmd_HookCmd("gl_texturemode", Draw_TextureMode_f);
	Cmd_HookCmd("screenshot", CL_ScreenShot_f);
}

byte *R_GetTexLoaderBuffer(int *bufsize)
{
	if(bufsize)
		*bufsize = sizeof(texloader_buffer);
	return texloader_buffer;
}

//Texture resampler

void GL_ResampleTexture(unsigned *in, int inwidth, int inheight, unsigned *out, int outwidth, int outheight)
{
	int i, j;
	unsigned *inrow, *inrow2;
	unsigned frac, fracstep;
	unsigned p1[1024], p2[1024];
	byte *pix1, *pix2, *pix3, *pix4;

	fracstep = inwidth * 0x10000 / outwidth;
	frac = fracstep >> 2;

	for (i = 0; i < outwidth; i++)
	{
		p1[i] = 4 * (frac >> 16);
		frac += fracstep;
	}

	frac = 3 * (fracstep >> 2);

	for (i = 0; i < outwidth; i++)
	{
		p2[i] = 4 * (frac >> 16);
		frac += fracstep;
	}

	for (i = 0; i < outheight; i++, out += outwidth)
	{
		inrow = in + inwidth * (int)((i + 0.25) * inheight / outheight);
		inrow2 = in + inwidth * (int)((i + 0.75) * inheight / outheight);

		frac = fracstep >> 1;

		for (j = 0; j < outwidth; j++)
		{
			pix1 = (byte *)inrow + p1[j];
			pix2 = (byte *)inrow + p2[j];
			pix3 = (byte *)inrow2 + p1[j];
			pix4 = (byte *)inrow2 + p2[j];
			((byte *)(out + j))[0] = (pix1[0] + pix2[0] + pix3[0] + pix4[0]) >> 2;
			((byte *)(out + j))[1] = (pix1[1] + pix2[1] + pix3[1] + pix4[1]) >> 2;
			((byte *)(out + j))[2] = (pix1[2] + pix2[2] + pix3[2] + pix4[2]) >> 2;
			((byte *)(out + j))[3] = (pix1[3] + pix2[3] + pix3[3] + pix4[3]) >> 2;
		}
	}
}

void GL_ResampleTexturePoint(unsigned char *in, int inwidth, int inheight, unsigned char *out, int outwidth, int outheight)
{
	int i, j;
	unsigned ufrac, vfrac;
	unsigned ufracstep, vfracstep;
	unsigned char *src, *dest;

	src = in;
	dest = out;
	ufracstep = inwidth * 0x10000 / outwidth;
	vfracstep = inheight * 0x10000 / outheight;

	vfrac = vfracstep >> 2;

	for (i = 0; i < outheight; i++, out += outwidth)
	{
		ufrac = ufracstep >> 2;

		for (j = 0; j < outwidth; j++)
		{
			*dest = src[ufrac >> 16];
			ufrac += ufracstep;
			dest++;
		}

		vfrac += vfracstep;
		src += inwidth * (vfrac >> 16);
		vfrac = vfrac & 0xFFFF;
	}
}

void GL_MipMap(byte *in, int width, int height)
{
	int i, j;
	byte *out;

	width <<=2;
	height >>= 1;
	out = in;

	for (i = 0; i < height; i++, in += width)
	{
		for (j = 0; j < width; j += 8, out += 4, in += 8)
		{
			out[0] = (in[0] + in[4] + in[width + 0] + in[width + 4]) >> 2;
			out[1] = (in[1] + in[5] + in[width + 1] + in[width + 5]) >> 2;
			out[2] = (in[2] + in[6] + in[width + 2] + in[width + 6]) >> 2;
			out[3] = (in[3] + in[7] + in[width + 3] + in[width + 7]) >> 2;
		}
	}
}

void ComputeScaledSize(int *wscale, int *hscale, int width, int height)
{
	int scaled_width, scaled_height;
	int max_size;

	max_size = max(128, (int)gl_max_size->value);

	for (scaled_width = 1; scaled_width < width; scaled_width <<= 1) {}

	if (gl_round_down->value > 0 && width < scaled_width && (gl_round_down->value == 1 || (scaled_width - width) > (scaled_width >> (int)gl_round_down->value)))
		scaled_width >>= 1;

	for (scaled_height = 1; scaled_height < height; scaled_height <<= 1) {}

	if (gl_round_down->value > 0 && height < scaled_height && (gl_round_down->value == 1 || (scaled_height - height) > (scaled_height >> (int)gl_round_down->value)))
		scaled_height >>= 1;

	if (wscale)
		*wscale = min(scaled_width >> (int)gl_picmip->value, max_size);

	if (hscale)
		*hscale = min(scaled_height >> (int)gl_picmip->value, max_size);
}

void GL_Upload32(unsigned int *data, int width, int height, qboolean mipmap, qboolean ansio)
{
	int iComponent, iFormat;
	int scaled_width, scaled_height;

	ComputeScaledSize(&scaled_width, &scaled_height, width, height);

	if (scaled_height * scaled_width > sizeof(scaled_buffer) / 4)
		Sys_ErrorEx("GL_Upload32: Texture is too large!");

	if(gl_loadtexture_format != GL_RGBA)
		Sys_ErrorEx("GL_Upload32: Only RGBA is supported!");

	iFormat = GL_RGBA;
	iComponent = GL_RGBA8;

	qglTexParameterf(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);

	if (mipmap)
	{
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min);
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);
		
	}
	else
	{
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_max);
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);
	}

	if(ansio)
	{
		if(gl_force_ansio)
			qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, gl_force_ansio);
		else
			qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max(min(gl_ansio->value, gl_max_ansio), 1));
	}
	else
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1);

	if (scaled_width == width && scaled_height == height)
	{
		if (!mipmap)
		{
			qglTexImage2D(GL_TEXTURE_2D, 0, iComponent, width, height, 0, iFormat, GL_UNSIGNED_BYTE, data);
			return;
		}

		memcpy(scaled_buffer, data, width * height * 4);
	}
	else
	{
		GL_ResampleTexture(data, width, height, (unsigned int *)scaled_buffer, scaled_width, scaled_height);
	}

	qglTexImage2D(GL_TEXTURE_2D, 0, iComponent, scaled_width, scaled_height, 0, iFormat, GL_UNSIGNED_BYTE, scaled_buffer);

	if (mipmap)
	{
		int miplevel = 0;

		while (scaled_width > 1 || scaled_height > 1)
		{
			GL_MipMap((byte *)scaled_buffer, scaled_width, scaled_height);

			scaled_width >>= 1;
			scaled_height >>= 1;

			if (scaled_width < 1)
				scaled_width = 1;

			if (scaled_height < 1)
				scaled_height = 1;

			miplevel++;

			qglTexImage2D(GL_TEXTURE_2D, miplevel, iComponent, scaled_width, scaled_height, 0, iFormat, GL_UNSIGNED_BYTE, scaled_buffer);
		}
	}
}

void GL_UploadDXT(byte *data, int width, int height, qboolean mipmap, qboolean ansio)
{
	if (mipmap)
	{
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min);
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);
		qglTexParameterf(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	}
	else
	{
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_max);
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);
		qglTexParameterf(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
	}

	if(ansio)
	{
		if(gl_force_ansio)
			qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, gl_force_ansio);
		else
			qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max(min(gl_ansio->value, gl_max_ansio), 1));
	}
	else
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1);

	qglCompressedTexImage2DARB(GL_TEXTURE_2D, 0, gl_loadtexture_format, width, height, 0, gl_loadtexture_size, data);
}

int GL_AllocTexture(const char *identifier, GL_TEXTURETYPE textureType, int width, int height, qboolean mipmap)
{
	int i;
	gltexture_t *glt;
	gltexture_t *slot;

	glt = NULL;

	char *pName = (char *)identifier;

	if (pName[0])
	{
		for (i = 0, slot = gltextures; i < *numgltextures; i++, slot++)
		{
			if (slot->servercount < 0)
			{
				if (!glt)
					glt = slot;

				break;
			}

			if (!strcmp(pName, slot->identifier))
			{
				if (width != slot->width || height != slot->height)
				{
					pName[3]++;
					continue;
				}

				if (slot->servercount > 0)
					slot->servercount = *gHostSpawnCount;

				currentglt = slot;
				return -slot->texnum;
			}
		}
	}
	else
	{
		gEngfuncs.Con_DPrintf("NULL Texture\n");
		return 0;
	}

	if (!glt)
	{
		glt = &gltextures[(*numgltextures)];
		(*numgltextures)++;

		if (*numgltextures >= MAX_GLTEXTURES)
		{
			gEngfuncs.Con_Printf("Texture Overflow: MAX_GLTEXTURES\n");
			return 0;
		}
	}

	if (!glt->texnum)
		glt->texnum = GL_GenTexture();

	strncpy(glt->identifier, identifier, sizeof(glt->identifier) - 1);
	glt->identifier[sizeof(glt->identifier) - 1] = 0;
	glt->width = width;
	glt->height = height;
	glt->mipmap = mipmap;

	if (textureType == GLT_WORLD)
		glt->servercount = *gHostSpawnCount;
	else
		glt->servercount = 0;

	glt->paletteIndex = -1;

	currentglt = glt;

	return glt->texnum;
}

int GL_LoadTextureEx(const char *identifier, GL_TEXTURETYPE textureType, int width, int height, byte *data, qboolean mipmap, qboolean ansio)
{
	int scaled_width, scaled_height;
	qboolean rescale;
	byte *pTexture;
	int texnum;

	texnum = GL_AllocTexture(identifier, textureType, width, height, mipmap);

	if(texnum < 0)
		return -texnum;
	if(texnum == 0)
		return 0;

	GL_Bind(texnum);
	(*currenttexture) = -1;

	if(gl_loadtexture_format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT || gl_loadtexture_format == GL_COMPRESSED_RGBA_S3TC_DXT3_EXT || gl_loadtexture_format == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT)
	{
		GL_UploadDXT(data, width, height, mipmap, ansio);
		return texnum;
	}

	ComputeScaledSize(&scaled_width, &scaled_height, width, height);
	rescale = (scaled_width == width && scaled_height == height) ? false : true;

	pTexture = data;
	if (!mipmap && rescale && scaled_width <= 128 && scaled_height <= 128)
	{
		GL_ResampleTexturePoint(data, width, height, scaled_buffer, scaled_width, scaled_height);
		pTexture = scaled_buffer;
	}

	if (pTexture)
	{
		GL_Upload32((unsigned *)data, width, height, mipmap, ansio);
	}

	return texnum;
}

DWORD ByteToUInt( byte *byte )
{
	DWORD iValue = byte[0];
	iValue += (byte[1]<<8);
	iValue += (byte[2]<<16);
	iValue += (byte[3]<<24);

	return iValue;
}

qboolean PowerOfTwo(int iWidth,int iHeight)
{
	int iWidthT = iWidth;
	while(iWidthT != 1)
	{
		if((iWidthT % 2) != 0) return false;
		iWidthT /=2;
	}

	int iHeightT = iHeight;
	while(iHeightT != 1)
	{
		if((iHeightT % 2) != 0) return false;
		iHeightT /=2;
	}
	return true;
}

int GL_LoadDDS(const char *path, byte *buf, size_t bufsize, int *width, int *height)
{
	dds_header_t *header;
	byte *pFile;

	if (width)
		*width = 0;

	if (height)
		*height = 0;

	pFile = (byte *)gEngfuncs.COM_LoadFile((char *)path, 5, NULL);

	if(!pFile)
	{
		gEngfuncs.Con_Printf("GL_LoadDDS: couldn't open file %s\n", path);
		return FALSE;
	}

	header = (dds_header_t *)pFile;

	DWORD iFlags = ByteToUInt(header->bFlags);
	DWORD iMagic = ByteToUInt(header->bMagic);
	DWORD iFourCC = ByteToUInt(header->bPFFourCC);
	DWORD iPFFlags = ByteToUInt(header->bPFFlags);
	DWORD iLinSize = ByteToUInt(header->bPitchOrLinearSize);
	DWORD iSize = ByteToUInt(header->bSize);

	int w = ByteToUInt(header->bWidth);
	int h = ByteToUInt(header->bHeight);

	if(!PowerOfTwo(w, h))
	{
		gEngfuncs.Con_Printf("GL_LoadDDS: texture %s size is not power of 2\n", path);
		gEngfuncs.COM_FreeFile(pFile);
		return FALSE;
	}

	if(iMagic != DDS_MAGIC)
	{
		gEngfuncs.COM_FreeFile(pFile);
		return FALSE;
	}

	if(iSize != 124)
	{
		gEngfuncs.COM_FreeFile(pFile);
		return FALSE;
	}

	if(!(iFlags & DDSD_PIXELFORMAT))
	{
		gEngfuncs.COM_FreeFile(pFile);
		return FALSE;
	}

	if(!(iFlags & DDSD_CAPS))
	{
		gEngfuncs.COM_FreeFile(pFile);
		return FALSE;
	}

	if(!(iPFFlags & DDPF_FOURCC))
	{
		gEngfuncs.COM_FreeFile(pFile);
		return FALSE;
	}

	if(iFourCC != D3DFMT_DXT1 && iFourCC != D3DFMT_DXT3 && iFourCC != D3DFMT_DXT5)
	{
		gEngfuncs.Con_Printf("GL_LoadDDS: %s nou supported, Only DXT1/3/5 are supported!\n", path);
		gEngfuncs.COM_FreeFile(pFile);
		return FALSE;
	}

	if(iLinSize > bufsize)
	{
		gEngfuncs.Con_Printf("GL_LoadDDS: Texture %s is too large!\n", path);
		gEngfuncs.COM_FreeFile(pFile);
		return FALSE;
	}

	memcpy(buf, (pFile+128), iLinSize);

	if(iFourCC == D3DFMT_DXT1)
		gl_loadtexture_format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
	else if(iFourCC == D3DFMT_DXT3)
		gl_loadtexture_format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
	else if(iFourCC == D3DFMT_DXT5)
		gl_loadtexture_format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;

	gl_loadtexture_size = iLinSize;

	*width = w;
	*height = h;

	gEngfuncs.COM_FreeFile(pFile);
	return TRUE;
}

void GL_ReadPNG(png_structp png_ptr, png_bytep data, png_size_t length)
{
	png_source *src = (png_source *)png_get_io_ptr(png_ptr);
	if(src->offset + length <= src->size)
	{
		memcpy(data, (PVOID)((DWORD)src->data+src->offset), length);
		src->offset += length;
	}
	else
		png_error(png_ptr, "GL_ReadPNG failed");
}

int GL_LoadPNG(const char *path, byte *buf, size_t bufsize, int *width, int *height)
{
	int iFileLength;
	byte *pFile;

	png_structp png_ptr;
	png_infop info_ptr;
	png_source src;
	png_bytep *row_pointers;
	int w, h, i, j, pos, block_size;
	int iBBP, iColorType;

	if (width)
		*width = 0;

	if (height)
		*height = 0;

	pFile = (byte *)gEngfuncs.COM_LoadFile((char *)path, 5, &iFileLength);

	if(!pFile)
	{
		gEngfuncs.Con_Printf("GL_LoadPNG: Couldn't open file %s\n", path);
		return FALSE;
	}

	if(png_sig_cmp(pFile, 0, 8))
    {
		gEngfuncs.Con_Printf("GL_LoadPNG: File %s is not a PNG image.\n", path);
		return FALSE;
    }

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	if(!png_ptr)
	{
		return FALSE;
	}
	info_ptr = png_create_info_struct(png_ptr);
	if(!info_ptr)
	{
		png_destroy_read_struct(&png_ptr, 0, 0);
		return FALSE;
	}
	if(setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, 0);
		return FALSE;
	}

	src.data = pFile;
	src.size = iFileLength;
	src.offset = 0;

	png_set_read_fn(png_ptr, &src, GL_ReadPNG);
	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND | PNG_TRANSFORM_STRIP_ALPHA, 0);
	w = png_get_image_width(png_ptr, info_ptr);
	h = png_get_image_height(png_ptr, info_ptr);
	iColorType = png_get_color_type(png_ptr, info_ptr);
	iBBP = png_get_bit_depth(png_ptr, info_ptr);

	row_pointers = png_get_rows(png_ptr, info_ptr);

	if(iColorType == PNG_COLOR_TYPE_RGB)
		block_size = 3;
	else if(iColorType == PNG_COLOR_TYPE_RGBA)
		block_size = 4;
	else
	{
		gEngfuncs.Con_Printf("GL_LoadDDS: %s not supported, Only RGB/RGBA are supported!\n", path);
		png_destroy_read_struct(&png_ptr, &info_ptr, 0);
		gEngfuncs.COM_FreeFile(pFile);
		return FALSE;
	}

	if((size_t)(w * h * 4) > bufsize)
	{
		gEngfuncs.Con_Printf("GL_LoadDDS: Texture %s is too large!\n", path);
		png_destroy_read_struct(&png_ptr, &info_ptr, 0);
		gEngfuncs.COM_FreeFile(pFile);
		return FALSE;
	}
	
	pos = 0;
	for( i = 0; i < h; i++ )
	{
		for( j = 0; j < w * block_size; j += block_size )
		{
			buf[pos++] = row_pointers[i][j + 0];//B
			buf[pos++] = row_pointers[i][j + 1];//G
			buf[pos++] = row_pointers[i][j + 2];//R
			buf[pos++] = (block_size == 3) ? 255 : row_pointers[i][j + 3];//Alpha
		}
	}

	*width = w;
	*height = h;

	gl_loadtexture_format = GL_RGBA;
	gl_loadtexture_size = pos;

	png_destroy_read_struct(&png_ptr, &info_ptr, 0);
	gEngfuncs.COM_FreeFile(pFile);
	return TRUE;
}

int R_LoadTextureEx(const char *path, const char *name, int *width, int *height, GL_TEXTURETYPE type, qboolean mipmap, qboolean ansio)
{
	int w, h;
	const char *extension = path + strlen(path) - 4;
	if(!stricmp(extension, ".dds"))
	{
		if(GL_LoadDDS(path, texloader_buffer, sizeof(texloader_buffer), &w, &h))
		{
			if(width)
				*width = w;
			if(height)
				*height = h;
			return GL_LoadTextureEx(name, type, w, h, texloader_buffer, mipmap, ansio);
		}
	}
	else if(!stricmp(extension, ".tga"))
	{
		gl_loadtexture_format = GL_RGBA;
		if(gRefFuncs.LoadTGA(path, texloader_buffer, sizeof(texloader_buffer), &w, &h))
		{
			if(width)
				*width = w;
			if(height)
				*height = h;
			return GL_LoadTextureEx(name, type, w, h, texloader_buffer, mipmap, ansio);
		}
	}
	else if(!stricmp(extension, ".png"))
	{
		gl_loadtexture_format = GL_RGBA;
		if(GL_LoadPNG(path, texloader_buffer, sizeof(texloader_buffer), &w, &h))
		{
			if(width)
				*width = w;
			if(height)
				*height = h;
			return GL_LoadTextureEx(name, type, w, h, texloader_buffer, mipmap, ansio);
		}
	}
	else
	{
		gEngfuncs.Con_Printf("R_LoadTextureEx: %s unsupported texture extension \"%s\".\n", path, extension);
		return 0;
	}
	gEngfuncs.Con_Printf("R_LoadTextureEx: failed to load texture %s.\n", path);
	return 0;
}

gltexture_t *R_GetCurrentGLTexture(void)
{
	return currentglt;
}

extern cvar_t *r_wsurf_decal;

texture_t *Draw_DecalTexture(int index)
{
	texture_t *t = gRefFuncs.Draw_DecalTexture(index);
	if(index < 0)
		return t;
	if(t->anim_next && r_wsurf_decal->value)
		return t->anim_next;

	return t;
}

void Draw_MiptexTexture(cachewad_t *wad, byte *data)
{
	gRefFuncs.Draw_MiptexTexture(wad, data);

	texture_t *t = (texture_t *)data;
	R_LinkDecalTexture(t);
}

void GL_WritePNG(png_structp png_ptr, png_bytep data, png_size_t length)
{
	png_dest *dst = (png_dest *)png_get_io_ptr(png_ptr);
	g_pFileSystem->Write(data, length, dst->fp);
}

void GL_FlushPNG(png_structp png_ptr)
{
	png_dest *dst = (png_dest *)png_get_io_ptr(png_ptr);
	g_pFileSystem->Flush(dst->fp);
}

qboolean GL_SavePNG(const char *file_name, int width, int height, byte *data)
{
    int i;
	png_dest dst;
    png_structp png_ptr;
    png_infop info_ptr;
    png_bytep *row_pointers;

	FileHandle_t fp = g_pFileSystem->Open(file_name, "wb");
    if (!fp)
    {
		gEngfuncs.Con_Printf("GL_SavePNG: Couldn't open %s for write\n", file_name);
        return FALSE;  
    }

    /* initialize stuff */  
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);  

    if (!png_ptr)  
    {
		gEngfuncs.Con_Printf("GL_SavePNG: png_create_write_struct failed");
		g_pFileSystem->Close(fp);
        return FALSE;  
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)  
    {  
		gEngfuncs.Con_Printf("GL_SavePNG: png_create_info_struct failed");
		g_pFileSystem->Close(fp);
		png_destroy_write_struct(&png_ptr, NULL);
		return FALSE;  
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {  
		gEngfuncs.Con_Printf("GL_SavePNG: png_set_write_fn failed ");
		g_pFileSystem->Close(fp);
		png_destroy_write_struct(&png_ptr, &info_ptr);
        return FALSE;  
    }

	dst.fp = fp;

	png_set_write_fn(png_ptr, &dst, GL_WritePNG, GL_FlushPNG);

	if (setjmp(png_jmpbuf(png_ptr)))  
    {  
		gEngfuncs.Con_Printf("GL_SavePNG: Error writing header");
		g_pFileSystem->Close(fp);
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return FALSE;
    }

    png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);  

	png_write_info(png_ptr, info_ptr);
  
	row_pointers = (png_bytepp)malloc(height * sizeof(png_bytep));
	if(!row_pointers)
    {  
		gEngfuncs.Con_Printf("GL_SavePNG: Error during allocate row_pointers");
		png_destroy_write_struct(&png_ptr, &info_ptr);
		g_pFileSystem->Close(fp);
        return FALSE;
    }

	for (i = 0; i < height; i++)
        row_pointers[height-i-1] = data + i * width * 3;//reverse image

    /* write bytes */  
    if (setjmp(png_jmpbuf(png_ptr)))
    {  
		gEngfuncs.Con_Printf("GL_SavePNG: Error during writing bytes");
		g_pFileSystem->Close(fp);
		png_destroy_write_struct(&png_ptr, &info_ptr);
		free(row_pointers);
        return FALSE;
    }

    png_write_image(png_ptr, row_pointers);
  
    if (setjmp(png_jmpbuf(png_ptr)))  
    {  
		gEngfuncs.Con_Printf("GL_SavePNG: Error during end of write");
		png_destroy_write_struct(&png_ptr, &info_ptr);
		free(row_pointers);
		g_pFileSystem->Close(fp);
		return FALSE;
    }

	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);
	free(row_pointers);
	g_pFileSystem->Close(fp);
	return TRUE;
}

qboolean GL_SaveTGA(const char *file_name, int width, int height, byte *data)
{
	FileHandle_t fp = g_pFileSystem->Open(file_name, "wb");
    if (!fp)
    {
		gEngfuncs.Con_Printf("GL_SaveTGA: Couldn't open %s for write\n", file_name);
		return FALSE;
    }
	tgaheader_t header;

	memset(&header, 0, sizeof(tgaheader_t));
	header.bImageType = 2;
	header.bWidth[0] = width & 255;
	header.bWidth[1] = width >> 8;
	header.bHeight[0] = height & 255;
	header.bHeight[1] = height >> 8;
	header.bPixelDepth = 24;

	g_pFileSystem->Write(&header, sizeof(tgaheader_t), fp);

	int nPixelSize = width * height * 3;
	int i;

	byte buf[3];
	byte *p = data;
	for (i = 0; i < nPixelSize; i += 3, p += 3)
	{
		buf[0] = p[2];
		buf[1] = p[1];
		buf[2] = p[0];
		g_pFileSystem->Write(buf, sizeof(buf), fp);
	}

	g_pFileSystem->Close(fp);
	return TRUE;
}

qboolean GL_SaveBMP(const char *file_name, int width, int height, byte *data)
{
	FileHandle_t fp = g_pFileSystem->Open(file_name, "wb");
    if (!fp)
    {
		gEngfuncs.Con_Printf("GL_SaveBMP: Couldn't open %s for write\n", file_name);
        return FALSE;  
    }

	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER bmih;

	const int OffBits = 54;

	bmfh.bfReserved1 = 0;  
	bmfh.bfReserved2 = 0;  
	bmfh.bfType = 0x4D42;
	bmfh.bfOffBits = OffBits;
	bmfh.bfSize = width * height * 3 + OffBits;
  
    memset(&bmih, 0 ,sizeof(BITMAPINFOHEADER));
    bmih.biSize = 40; 
    bmih.biPlanes = 1;
    bmih.biSizeImage = width * height * 3;
    bmih.biBitCount = 24;
    bmih.biCompression = 0;
    bmih.biWidth = width;
    bmih.biHeight = height;
  
	g_pFileSystem->Write(&bmfh, sizeof(BITMAPFILEHEADER), fp);
	g_pFileSystem->Write(&bmih, sizeof(BITMAPINFOHEADER), fp);

	byte buf[3];
	byte *p = data;
	for( int i = height - 1; i >= 0; --i)  
	{
		for( int j = 0; j < width; ++j, p += 3)  
		{
			buf[0] = p[2];
			buf[1] = p[1];
			buf[2] = p[0];
			g_pFileSystem->Write(buf, sizeof(buf), fp);
		}
	}

	g_pFileSystem->Close(fp);
	return TRUE;
}