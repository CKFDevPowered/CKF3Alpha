#include "gl_local.h"
#include "command.h"

//For PNG Support
extern "C"
{
#include "libpng/png.h"
#include "libpng/pnginfo.h"

#define MEM_SRCDST_SUPPORTED

#include "libjpeg/jpeglib.h"
#include "libjpeg/jerror.h"
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

//PNG
#pragma comment(lib,"libpng/zlib.lib")
#pragma comment(lib,"libpng/libpng.lib")

//JPEG

#pragma comment(lib,"libjpeg/jpeg-static.lib")

struct jpeg_my_error_mgr
{
	struct jpeg_error_mgr pub;    /* "public" fields */
	jmp_buf setjmp_buffer;        /* for return to caller */
};

typedef struct jpeg_my_error_mgr *my_jpeg_error_ptr;

//GL Start

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

	if (g_pMetaSave->pEngineFuncs->Cmd_Argc() == 1)
	{
		for (i = 0; i < 6; i++)
		{
			if (gl_filter_min == modes[i].minimize)
			{
				g_pMetaSave->pEngineFuncs->Con_Printf("%s\n", modes[i].name);
				return;
			}
		}

		g_pMetaSave->pEngineFuncs->Con_Printf("current filter is unknown???\n");
		return;
	}

	for (i = 0; i < 6; i++)
	{
		if (!stricmp(modes[i].name, g_pMetaSave->pEngineFuncs->Cmd_Argv(1)))
			break;
	}

	if (i == 6)
	{
		g_pMetaSave->pEngineFuncs->Con_Printf("bad filter name\n");
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
				g_pMetaSave->pEngineFuncs->pfnClientCmd(cmd);
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
		g_pMetaSave->pEngineFuncs->Con_DPrintf("NULL Texture\n");
		return 0;
	}

	if (!glt)
	{
		glt = &gltextures[(*numgltextures)];
		(*numgltextures)++;

		if (*numgltextures >= MAX_GLTEXTURES)
		{
			g_pMetaSave->pEngineFuncs->Con_Printf("Texture Overflow: MAX_GLTEXTURES\n");
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

int LoadBMP(const char *szFilename, byte *buffer, int bufferSize, int *width, int *height)
{
	FileHandle_t file = g_pFileSystem->Open(szFilename, "rb");

	if (!file)
	{
		//g_pMetaSave->pEngineFuncs->Con_Printf("LoadBMP: Cannot open file %s for reading\n", szFilename);
		return FALSE;
	}

	BITMAPFILEHEADER bmfHeader;
	LPBITMAPINFO lpbmi;
	DWORD dwFileSize = g_pFileSystem->Size(file);

	if (!g_pFileSystem->Read(&bmfHeader, sizeof(bmfHeader), file))
	{
		*width = 0;
		*height = 0;

		g_pFileSystem->Close(file);
		g_pMetaSave->pEngineFuncs->Con_Printf("LoadBMP: File %s has no BMP header.\n", szFilename);
		return FALSE;
	}

	if (bmfHeader.bfType == DIB_HEADER_MARKER)
	{
		DWORD dwBitsSize = dwFileSize - sizeof(bmfHeader);

		HGLOBAL hDIB = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, dwBitsSize);
		char *pDIB = (LPSTR)::GlobalLock((HGLOBAL)hDIB);

		if (!g_pFileSystem->Read(pDIB, dwBitsSize, file))
		{
			::GlobalUnlock(hDIB);
			::GlobalFree((HGLOBAL)hDIB);

			*width = 0;
			*height = 0;

			g_pFileSystem->Close(file);
			g_pMetaSave->pEngineFuncs->Con_Printf("LoadBMP: File %s has no DIB info.\n", szFilename);
			return FALSE;
		}

		lpbmi = (LPBITMAPINFO)pDIB;

		if (width)
			*width = lpbmi->bmiHeader.biWidth;

		if (height)
			*height = lpbmi->bmiHeader.biHeight;

		unsigned char *rgba = (unsigned char *)(pDIB + sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD));

		for (int j = 0; j < lpbmi->bmiHeader.biHeight; j++)
		{
			for (int i = 0; i < lpbmi->bmiHeader.biWidth; i++)
			{
				int y = (lpbmi->bmiHeader.biHeight - j - 1);

				int offs = (y * lpbmi->bmiHeader.biWidth + i);
				int offsdest = (j * lpbmi->bmiHeader.biWidth + i) * 4;
				unsigned char *src = rgba + offs;
				unsigned char *dst = buffer + offsdest;

				dst[0] = lpbmi->bmiColors[*src].rgbRed;
				dst[1] = lpbmi->bmiColors[*src].rgbGreen;
				dst[2] = lpbmi->bmiColors[*src].rgbBlue;
				dst[3] = 255;
			}
		}

		::GlobalUnlock(hDIB);
		::GlobalFree((HGLOBAL)hDIB);
	}

	g_pFileSystem->Close(file);

	return TRUE;
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

int LoadDDS(const char *szFilename, byte *buf, int bufsize, int *width, int *height)
{
	FileHandle_t fp;
	dds_header_t header;
	int fileSize;

	if (width)
		*width = 0;

	if (height)
		*height = 0;

	fp = g_pFileSystem->Open(szFilename, "rb");

	if(!fp)
	{
		//g_pMetaSave->pEngineFuncs->Con_DPrintf("LoadDDS: Cannot open file %s for reading.\n", szFilename);
		return FALSE;
	}

	fileSize = g_pFileSystem->Size(fp);

	if(!g_pFileSystem->Read((void *)&header, sizeof(dds_header_t), fp))
	{
		g_pMetaSave->pEngineFuncs->Con_Printf("LoadDDS: File %s is not a DXT image.\n", szFilename);
		return FALSE;
	}

	DWORD iFlags = ByteToUInt(header.bFlags);
	DWORD iMagic = ByteToUInt(header.bMagic);
	DWORD iFourCC = ByteToUInt(header.bPFFourCC);
	DWORD iPFFlags = ByteToUInt(header.bPFFlags);
	DWORD iLinSize = ByteToUInt(header.bPitchOrLinearSize);
	DWORD iSize = ByteToUInt(header.bSize);

	int w = ByteToUInt(header.bWidth);
	int h = ByteToUInt(header.bHeight);

	//if(!PowerOfTwo(w, h))
	//{
	//	g_pMetaSave->pEngineFuncs->Con_Printf("LoadDDS: Texture %s's size is not power of 2.\n", szFilename);
	//	if(fp) g_pFileSystem->Close(fp);
	//	return FALSE;
	//}

	if(iMagic != DDS_MAGIC)
	{
		g_pMetaSave->pEngineFuncs->Con_Printf("LoadDDS: File %s is not a DXT image.\n", szFilename);
		if(fp) g_pFileSystem->Close(fp);
		return FALSE;
	}

	if(iSize != 124)
	{
		g_pMetaSave->pEngineFuncs->Con_Printf("LoadDDS: File %s is not a DXT image.\n", szFilename);
		if(fp) g_pFileSystem->Close(fp);
		return FALSE;
	}

	if(!(iFlags & DDSD_PIXELFORMAT))
	{
		g_pMetaSave->pEngineFuncs->Con_Printf("LoadDDS: File %s is not a DXT image.\n", szFilename);
		if(fp) g_pFileSystem->Close(fp);
		return FALSE;
	}

	if(!(iFlags & DDSD_CAPS))
	{
		g_pMetaSave->pEngineFuncs->Con_Printf("LoadDDS: File %s is not a DXT image.\n", szFilename);
		if(fp) g_pFileSystem->Close(fp);
		return FALSE;
	}

	if(!(iPFFlags & DDPF_FOURCC))
	{
		g_pMetaSave->pEngineFuncs->Con_Printf("LoadDDS: File %s is not a DXT image.\n", szFilename);
		if(fp) g_pFileSystem->Close(fp);
		return FALSE;
	}

	if(iFourCC != D3DFMT_DXT1 && iFourCC != D3DFMT_DXT3 && iFourCC != D3DFMT_DXT5)
	{
		g_pMetaSave->pEngineFuncs->Con_Printf("LoadDDS: Texture %s is not DXT1/3/5 format!\n", szFilename);
		if(fp) g_pFileSystem->Close(fp);
		return FALSE;
	}

	if((int)iLinSize > bufsize)
	{
		g_pMetaSave->pEngineFuncs->Con_Printf("LoadDDS: Texture %s is too large!\n", szFilename);
		if(fp) g_pFileSystem->Close(fp);
		return FALSE;
	}

	g_pFileSystem->Seek(fp, 128, FILESYSTEM_SEEK_HEAD);

	if(!g_pFileSystem->Read((void *)buf, iLinSize, fp))
	{
		g_pMetaSave->pEngineFuncs->Con_Printf("LoadDDS: File %s is corrupted.\n", szFilename);
		if(fp) g_pFileSystem->Close(fp);
		return FALSE;
	}

	if(iFourCC == D3DFMT_DXT1)
		gl_loadtexture_format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
	else if(iFourCC == D3DFMT_DXT3)
		gl_loadtexture_format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
	else if(iFourCC == D3DFMT_DXT5)
		gl_loadtexture_format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;

	gl_loadtexture_size = iLinSize;

	if(width)
		*width = w;
	if(height)
		*height = h;

	if(fp) g_pFileSystem->Close(fp);
	return TRUE;
}

void ReadPNGCallBack(png_structp png_ptr, png_bytep data, png_size_t length)
{
	png_source *src = (png_source *)png_get_io_ptr(png_ptr);
	if(src->offset + length <= src->size)
	{
		g_pFileSystem->Read(data, length, src->fp);
		src->offset += length;
	}
	else
		png_error(png_ptr, "ReadPNGCallBack failed");
}

int LoadPNG(const char *szFilename, byte *buf, int bufsize, int *width, int *height)
{
	FileHandle_t fp;
	int fileLength;
	byte fileHeader[8];

	png_structp png_ptr;
	png_infop info_ptr;
	png_source src;
	png_bytep *row_pointers;
	int w, h, i, j, pos, block_size;
	int bbp, colorType;

	if (width)
		*width = 0;

	if (height)
		*height = 0;

	fp = g_pFileSystem->Open((char *)szFilename, "rb");
	if(!fp)
	{
		//g_pMetaSave->pEngineFuncs->Con_DPrintf("LoadPNG: Couldn't open file %s\n", szFilename);
		return FALSE;
	}

	fileLength = g_pFileSystem->Size(fp);

	if(!g_pFileSystem->Read(fileHeader, 8, fp) || png_sig_cmp(fileHeader, 0, 8))
    {
		g_pMetaSave->pEngineFuncs->Con_Printf("LoadPNG: File %s is not a PNG image.\n", szFilename);
		return FALSE;
    }

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	if(!png_ptr)
	{
		if(fp) g_pFileSystem->Close(fp);

		return FALSE;
	}

	png_set_sig_bytes(png_ptr, 8);

	info_ptr = png_create_info_struct(png_ptr);
	if(!info_ptr)
	{
		if(fp) g_pFileSystem->Close(fp);
		png_destroy_read_struct(&png_ptr, 0, 0);
		return FALSE;
	}

	if(setjmp(png_jmpbuf(png_ptr)))
	{
		if(fp) g_pFileSystem->Close(fp);
		png_destroy_read_struct(&png_ptr, &info_ptr, 0);
		return FALSE;
	}

	src.fp = fp;
	src.size = fileLength;
	src.offset = 0;

	png_set_read_fn(png_ptr, &src, ReadPNGCallBack);
	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND | PNG_TRANSFORM_STRIP_ALPHA, 0);
	w = png_get_image_width(png_ptr, info_ptr);
	h = png_get_image_height(png_ptr, info_ptr);
	colorType = png_get_color_type(png_ptr, info_ptr);
	bbp = png_get_bit_depth(png_ptr, info_ptr);

	row_pointers = png_get_rows(png_ptr, info_ptr);

	if(colorType == PNG_COLOR_TYPE_RGB)
	{
		block_size = 3;
	}
	else if(colorType == PNG_COLOR_TYPE_RGBA)
	{
		block_size = 4;
	}
	else
	{
		g_pMetaSave->pEngineFuncs->Con_Printf("LoadPNG: %s not supported, Only RGB/RGBA are supported!\n", szFilename);

		png_destroy_read_struct(&png_ptr, &info_ptr, 0);
		if(fp) g_pFileSystem->Close(fp);
		return FALSE;
	}

	if(w * h * 4 > bufsize)
	{
		g_pMetaSave->pEngineFuncs->Con_Printf("LoadPNG: Texture %s is too large!\n", szFilename);

		png_destroy_read_struct(&png_ptr, &info_ptr, 0);
		if(fp) g_pFileSystem->Close(fp);
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

	if(width)
		*width = w;
	if(height)
		*height = h;

	gl_loadtexture_format = GL_RGBA;
	gl_loadtexture_size = pos;

	png_destroy_read_struct(&png_ptr, &info_ptr, 0);

	if(fp) g_pFileSystem->Close(fp);
	return TRUE;
}

METHODDEF(void) JPEGErrorCallback(j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_jpeg_error_ptr myerr = (my_jpeg_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  //(*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}

int LoadJPEG(const char *szFilename, byte *buffer, int bufferSize, int *width, int *height)
{
	FileHandle_t fp = g_pFileSystem->Open(szFilename, "rb");
    if (!fp)
    {
        return FALSE;  
    }

	struct jpeg_decompress_struct cinfo;
	struct jpeg_my_error_mgr jerr;
	JSAMPARRAY jbuffer;
	int row_stride, outRowStride;
	byte *outBuffer;
	byte *fileBuffer = NULL;
	int outSize;

	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = JPEGErrorCallback;

	if (setjmp(jerr.setjmp_buffer))
	{
		g_pMetaSave->pEngineFuncs->Con_Printf("LoadJPEG: We encountered an error while reading %s!\n", szFilename);

		jpeg_destroy_decompress(&cinfo);
		if(fileBuffer) free(fileBuffer);
		if(fp) g_pFileSystem->Close(fp);		
		return FALSE;
	}

	jpeg_create_decompress(&cinfo);

	int fileSize = g_pFileSystem->Size(fp);

	fileBuffer = (byte *)malloc(fileSize);

	g_pFileSystem->Read(fileBuffer, fileSize, fp);

	jpeg_mem_src(&cinfo, fileBuffer, fileSize);

	jpeg_read_header(&cinfo, TRUE);

	jpeg_start_decompress(&cinfo);

	outSize = cinfo.output_width * cinfo.output_height * 4;
	
	if(outSize > bufferSize)
	{
		g_pMetaSave->pEngineFuncs->Con_Printf("LoadJPEG: Texture %s is too large!\n", szFilename);

		jpeg_destroy_decompress(&cinfo);
		if(fileBuffer) free(fileBuffer);
		if(fp) g_pFileSystem->Close(fp);
		return FALSE;
	}

	outBuffer = buffer ;
	outRowStride = cinfo.output_width * 4;

	row_stride = cinfo.output_width * cinfo.output_components;

	jbuffer = (*cinfo.mem->alloc_sarray)
		 ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

	while (cinfo.output_scanline < cinfo.output_height)
	{
		outBuffer += outRowStride;
		jpeg_read_scanlines(&cinfo, jbuffer, 1);
		if(cinfo.output_components == 4)
		{
			memcpy(outBuffer, jbuffer[0], row_stride);
		}
		else if(cinfo.output_components == 3)
		{
			byte *out2 = jbuffer[0];
			byte *outBuffer2 = outBuffer;
			for( int i = 0; i < (int)cinfo.output_width; ++i )
			{
				outBuffer2[0] = out2[0];
				outBuffer2[1] = out2[1];
				outBuffer2[2] = out2[2];
				outBuffer2[3] = 255;
				outBuffer2 += 4;
				out2 += 3;
			}
		}
	}

	jpeg_finish_decompress(&cinfo);

	if(width)
		*width = cinfo.output_width;
	if(height)
		*height = cinfo.output_height;

	g_pFileSystem->Close(fp);

	jpeg_destroy_decompress(&cinfo);	
	return TRUE;
}

int LoadTGA(const char *szFilename, byte *buffer, int bufferSize, int *width, int *height)
{
	return gRefFuncs.LoadTGA(szFilename, buffer, bufferSize, width, height);
}

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
	if (src == path || *src == '\\' || *src == '/' )
	{		
		return NULL;  // no extension
	}

	return src;
}

int R_LoadTextureEx(const char *path, const char *name, int *width, int *height, GL_TEXTURETYPE type, qboolean mipmap, qboolean ansio)
{
	int w, h;
	const char *extension = V_GetFileExtension(path);
	if(!extension)
	{
		g_pMetaSave->pEngineFuncs->Con_Printf("R_LoadTextureEx: File %s has no extension \"%s\".\n", path);
		return 0;
	}
	if(!stricmp(extension, "bmp"))
	{
		gl_loadtexture_format = GL_RGBA;
		if(LoadBMP(path, texloader_buffer, sizeof(texloader_buffer), &w, &h))
		{
			if(width)
				*width = w;
			if(height)
				*height = h;
			return GL_LoadTextureEx(name, type, w, h, texloader_buffer, mipmap, ansio);
		}
	}
	else if(!stricmp(extension, "dds"))
	{
		if(LoadDDS(path, texloader_buffer, sizeof(texloader_buffer), &w, &h))
		{
			if(width)
				*width = w;
			if(height)
				*height = h;
			return GL_LoadTextureEx(name, type, w, h, texloader_buffer, mipmap, ansio);
		}
	}
	else if(!stricmp(extension, "tga"))
	{
		gl_loadtexture_format = GL_RGBA;
		if(LoadTGA(path, texloader_buffer, sizeof(texloader_buffer), &w, &h))
		{
			if(width)
				*width = w;
			if(height)
				*height = h;
			return GL_LoadTextureEx(name, type, w, h, texloader_buffer, mipmap, ansio);
		}
	}
	else if(!stricmp(extension, "png"))
	{
		gl_loadtexture_format = GL_RGBA;
		if(LoadPNG(path, texloader_buffer, sizeof(texloader_buffer), &w, &h))
		{
			if(width)
				*width = w;
			if(height)
				*height = h;
			return GL_LoadTextureEx(name, type, w, h, texloader_buffer, mipmap, ansio);
		}
	}
	else if(!stricmp(extension, ".jpg") || !stricmp(extension, ".jpeg"))
	{
		gl_loadtexture_format = GL_RGBA;
		if(LoadJPEG(path, texloader_buffer, sizeof(texloader_buffer), &w, &h))
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
		g_pMetaSave->pEngineFuncs->Con_Printf("R_LoadTextureEx: %s unsupported texture format.\n", path);
		return 0;
	}

	g_pMetaSave->pEngineFuncs->Con_Printf("R_LoadTextureEx: Failed to load texture %s.\n", path);
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

void WritePNGCallBack(png_structp png_ptr, png_bytep data, png_size_t length)
{
	png_dest *dst = (png_dest *)png_get_io_ptr(png_ptr);
	g_pFileSystem->Write(data, length, dst->fp);
}

void FlushPNGCallBack(png_structp png_ptr)
{
	png_dest *dst = (png_dest *)png_get_io_ptr(png_ptr);
	g_pFileSystem->Flush(dst->fp);
}

int SavePNG(const char *file_name, int width, int height, byte *data)
{
    int i;
	png_dest dst;
    png_structp png_ptr;
    png_infop info_ptr;
    png_bytep *row_pointers;

	FileHandle_t fp = g_pFileSystem->Open(file_name, "wb");
    if (!fp)
    {
		g_pMetaSave->pEngineFuncs->Con_Printf("SavePNG: Cannot open %s for writing.\n", file_name);
        return FALSE;  
    }

    /* initialize stuff */  
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);  

    if (!png_ptr)  
    {
		g_pMetaSave->pEngineFuncs->Con_Printf("SavePNG: png_create_write_struct failed");
		g_pFileSystem->Close(fp);
        return FALSE;  
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)  
    {  
		g_pMetaSave->pEngineFuncs->Con_Printf("SavePNG: png_create_info_struct failed");
		g_pFileSystem->Close(fp);
		png_destroy_write_struct(&png_ptr, NULL);
		return FALSE;  
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {  
		g_pMetaSave->pEngineFuncs->Con_Printf("SavePNG: png_set_write_fn failed ");
		g_pFileSystem->Close(fp);
		png_destroy_write_struct(&png_ptr, &info_ptr);
        return FALSE;  
    }

	dst.fp = fp;

	png_set_write_fn(png_ptr, &dst, WritePNGCallBack, FlushPNGCallBack);

	if (setjmp(png_jmpbuf(png_ptr)))  
    {  
		g_pMetaSave->pEngineFuncs->Con_Printf("SavePNG: Error writing header");
		g_pFileSystem->Close(fp);
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return FALSE;
    }

    png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);  

	png_write_info(png_ptr, info_ptr);
  
	row_pointers = (png_bytepp)malloc(height * sizeof(png_bytep));
	if(!row_pointers)
    {  
		g_pMetaSave->pEngineFuncs->Con_Printf("SavePNG: Error during allocate row_pointers");
		png_destroy_write_struct(&png_ptr, &info_ptr);
		g_pFileSystem->Close(fp);
        return FALSE;
    }

	for (i = 0; i < height; i++)
        row_pointers[height-i-1] = data + i * width * 3;//reverse image

    /* write bytes */  
    if (setjmp(png_jmpbuf(png_ptr)))
    {  
		g_pMetaSave->pEngineFuncs->Con_Printf("SavePNG: Error during writing bytes");
		g_pFileSystem->Close(fp);
		png_destroy_write_struct(&png_ptr, &info_ptr);
		free(row_pointers);
        return FALSE;
    }

    png_write_image(png_ptr, row_pointers);
  
    if (setjmp(png_jmpbuf(png_ptr)))  
    {  
		g_pMetaSave->pEngineFuncs->Con_Printf("SavePNG: Error during end of write");
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

int SaveTGA(const char *file_name, int width, int height, byte *data)
{
	FileHandle_t fp = g_pFileSystem->Open(file_name, "wb");
    if (!fp)
    {
		g_pMetaSave->pEngineFuncs->Con_Printf("SaveTGA: Cannot open %s for writing.\n", file_name);
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

int SaveBMP(const char *file_name, int width, int height, byte *data)
{
	FileHandle_t fp = g_pFileSystem->Open(file_name, "wb");
    if (!fp)
    {
		g_pMetaSave->pEngineFuncs->Con_Printf("SaveBMP: Cannot open %s for writing.\n", file_name);
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

int SaveJPEG(const char *file_name, int width, int height, byte *data)
{
	FileHandle_t fp = g_pFileSystem->Open(file_name, "wb");
    if (!fp)
    {
		g_pMetaSave->pEngineFuncs->Con_Printf("SaveJPEG: Cannot open %s for writing.\n", file_name);
        return FALSE;  
    }

	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	JSAMPROW row_pointer[1];
	int row_stride;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);

	byte *outBuffer = NULL;

	unsigned long outSize = 0;

	jpeg_mem_dest(&cinfo, &outBuffer, &outSize);

	cinfo.image_width = width;
	cinfo.image_height = height;
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_EXT_RGB;

	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, 70, TRUE);

	jpeg_start_compress(&cinfo, TRUE);

	row_stride = width * 3;

	while(cinfo.next_scanline < cinfo.image_height)
	{
		/* jpeg_write_scanlines expects an array of pointers to scanlines.
		* Here the array is only one element long, but you could pass
		* more than one scanline at a time if that's more convenient.
		*/
		row_pointer[0] = &data[(cinfo.image_height - 1 - cinfo.next_scanline) * row_stride];
		jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	jpeg_finish_compress(&cinfo);

	g_pFileSystem->Write(outBuffer, outSize, fp);
	g_pFileSystem->Close(fp);
	if(outBuffer) free(outBuffer);

	jpeg_destroy_compress(&cinfo);
	return TRUE;
}