#include <metahook.h>
#include "LoadBMP.h"
#include "plugins.h"
#include "perf_counter.h"
#include "cvardef.h"
#include "developer.h"
#include "console.h"

int LoadBMP(const char *szFilename, byte *buffer, int bufferSize, int *width, int *height)
{
	FileHandle_t file = g_pFileSystem->Open(szFilename, "rb");

	if (!file)
		return FALSE;

	bool debugTime = (developer && (int)developer->value > 2);
	double startTime;

	if (debugTime)
		startTime = gPerformanceCounter.GetCurTime();

	BITMAPFILEHEADER bmfHeader;
	LPBITMAPINFO lpbmi;
	DWORD dwFileSize = g_pFileSystem->Size(file);

	if (!g_pFileSystem->Read(&bmfHeader, sizeof(bmfHeader), file))
	{
		*width = 0;
		*height = 0;

		g_pFileSystem->Close(file);
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

	if (debugTime)
		Con_Printf("LoadBMP: loaded BMP %s in time %.4f sec.\n", szFilename, gPerformanceCounter.GetCurTime() - startTime);

	return TRUE;
}