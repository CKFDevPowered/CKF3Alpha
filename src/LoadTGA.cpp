#include <metahook.h>
#include "configs.h"
#include "LoadTGA.h"
#include "plugins.h"
#include "hooks.h"
#include "perf_counter.h"
#include "cvardef.h"
#include "developer.h"
#include "console.h"
#include "ref_int.h"

#pragma pack(1)

typedef struct _TargaHeader
{;
	unsigned char id_length, colormap_type, image_type;
	unsigned short colormap_index, colormap_length;
	unsigned char colormap_size;
	unsigned short x_origin, y_origin, width, height;
	unsigned char pixel_size, attributes;
}
TargaHeader;

#pragma pack()

int LoadTGA(const char *szFilename, byte *buffer, int bufferSize, int *width, int *height)
{
	//return g_pfnLoadTGA(szFilename, buffer, bufferSize, width, height);
	return gRefExports.LoadTGA(szFilename, buffer, bufferSize, width, height);
}

bool LoadTGA2(const char *szFilename, byte *buffer, int bufferSize, int *width, int *height)
{
	bool debugTime = (developer && (int)developer->value > 2);
	double startTime;

	if (debugTime)
		startTime = gPerformanceCounter.GetCurTime();

	FileHandle_t fp = g_pFileSystem->Open(szFilename, "rb");

	if (!fp)
		return FALSE;

	int columns, rows, numPixels;
	byte *pixbuf;
	int row, column;
	TargaHeader header;

	if (!g_pFileSystem->Read(&header, sizeof(header), fp))
	{
		*width = 0;
		*height = 0;

		g_pFileSystem->Close(fp);
		return FALSE;
	}

	if (header.image_type != 2 && header.image_type != 10)
		Con_DPrintf("LoadTGA2: Only type 2 and 10 targa RGB images supported\n");

	if (header.colormap_type != 0 || (header.pixel_size != 32 && header.pixel_size != 24))
		Con_DPrintf("LoadTGA2: Only 32 or 24 bit images supported (no colormaps)\n");

	columns = header.width;
	rows = header.height;
	numPixels = columns * rows;

	*width = header.width;
	*height = header.height;

	if (header.id_length != 0)
		g_pFileSystem->Seek(fp, header.id_length, FILESYSTEM_SEEK_CURRENT);

	int savepos = g_pFileSystem->Tell(fp);
	int filesize = g_pFileSystem->Size(fp);

	int size = filesize - savepos;
	byte *freebuf = (byte *)malloc(size);
	byte *fbuffer = freebuf;
	int readcount = g_pFileSystem->Read(fbuffer, size, fp);
	g_pFileSystem->Close(fp);

	if (!readcount)
	{
		free(freebuf);
		return FALSE;
	}

	if (header.image_type == 2)
	{
		for (row = rows - 1; row >= 0; row--)
		{
			pixbuf = buffer + row * columns * 4;

			for (column = 0; column < columns; column++)
			{
				register unsigned char red, green, blue, alphabyte;

				switch (header.pixel_size)
				{
					case 24:
					{
						blue = fbuffer[0];
						green = fbuffer[1];
						red = fbuffer[2];
						pixbuf[0] = red;
						pixbuf[1] = green;
						pixbuf[2] = blue;
						pixbuf[3] = 255;
						fbuffer += 3;
						pixbuf += 4;
						break;
					}

					case 32:
					{
						blue = fbuffer[0];
						green = fbuffer[1];
						red = fbuffer[2];
						alphabyte = fbuffer[3];
						pixbuf[0] = red;
						pixbuf[1] = green;
						pixbuf[2] = blue;
						pixbuf[3] = alphabyte;
						fbuffer += 4;
						pixbuf += 4;
						break;
					}
				}
			}
		}
	}
	else if (header.image_type == 10)
	{
		register unsigned char red, green, blue, alphabyte;
		unsigned char packetHeader, packetSize, j;

		for (row = rows - 1; row >= 0; row--)
		{
			pixbuf = buffer + row * columns * 4;

			for (column = 0; column < columns; )
			{
				packetHeader = *fbuffer++;
				packetSize = 1 + (packetHeader & 0x7F);

				if (packetHeader & 0x80)
				{
					switch (header.pixel_size)
					{
						case 24:
						{
							blue = fbuffer[0];
							green = fbuffer[1];
							red = fbuffer[2];
							alphabyte = 255;
							fbuffer += 3;
							break;
						}

						case 32:
						{
							blue = fbuffer[0];
							green = fbuffer[1];
							red = fbuffer[2];
							alphabyte = fbuffer[3];
							fbuffer += 4;
							break;
						}
					}

					for (j = 0; j < packetSize; j++)
					{
						pixbuf[0] = red;
						pixbuf[1] = green;
						pixbuf[2] = blue;
						pixbuf[3] = alphabyte;
						pixbuf += 4;
						column++;

						if (column == columns)
						{
							column = 0;

							if (row > 0)
								row--;
							else
								goto breakOut;

							pixbuf = buffer + row * columns * 4;
						}
					}
				}
				else
				{
					for (j = 0; j < packetSize; j++)
					{
						switch (header.pixel_size)
						{
							case 24:
							{
								blue = fbuffer[0];
								green = fbuffer[1];
								red = fbuffer[2];
								pixbuf[0] = red;
								pixbuf[1] = green;
								pixbuf[2] = blue;
								pixbuf[3] = 255;
								fbuffer += 3;
								pixbuf += 4;
								break;
							}

							case 32:
							{
								blue = fbuffer[0];
								green = fbuffer[1];
								red = fbuffer[2];
								alphabyte = fbuffer[3];
								pixbuf[0] = red;
								pixbuf[1] = green;
								pixbuf[2] = blue;
								pixbuf[3] = alphabyte;
								fbuffer += 4;
								pixbuf += 4;
								break;
							}
						}

						column++;

						if (column == columns)
						{
							column = 0;

							if (row > 0)
								row--;
							else
								goto breakOut;

							pixbuf = buffer + row * columns * 4;
						}
					}
				}
			}

			breakOut:;
		}
	}

	free(freebuf);

	if (debugTime)
		Con_Printf("LoadTGA2: loaded TGA %s in time %.4f sec.\n", szFilename, gPerformanceCounter.GetCurTime() - startTime);

	return TRUE;
}

//bool GetTGASize(const char *szFilename, int *width, int *height)
//{
//	FileHandle_t fp = g_pFileSystem->Open(szFilename, "rb");
//
//	if (!fp)
//		return FALSE;
//
//	TargaHeader header;
//
//	if (!g_pFileSystem->Read(&header, sizeof(header), fp))
//	{
//		*width = 0;
//		*height = 0;
//	}
//	else
//	{
//		*width = header.width;
//		*height = header.height;
//	}
//
//	g_pFileSystem->Close(fp);
//	return TRUE;
//}
//
//bool WriteTGA(byte *pixels, int width, int height, const char *szFilename)
//{
//	TargaHeader header;
//	memset(&header, 0, sizeof(header));
//
//	header.width = width;
//	header.height = height;
//	header.pixel_size = 24;
//	header.attributes = 0x20;
//	header.image_type = 2;
//
//	char filename[MAX_PATH];
//	sprintf(filename, "%s/%s", gConfigs.szGameLangDir, szFilename);
//
//	FILE *fp = fopen(filename, "wb");
//
//	if (!fp)
//		return FALSE;
//
//	if (!fwrite(&header, sizeof(header), 1, fp))
//	{
//		fclose(fp);
//		return FALSE;
//	}
//
//	int numpixels = width * height;
//
//	for (int i = 0; i < numpixels; i++)
//	{
//		register unsigned char color[3];
//
//		color[0] = pixels[2];
//		color[1] = pixels[1];
//		color[2] = pixels[0];
//		pixels += 3;
//
//		if (!fwrite(color, 3, 1, fp))
//		{
//			fclose(fp);
//			return FALSE;
//		}
//	}
//
//	fclose(fp);
//	return TRUE;
//}