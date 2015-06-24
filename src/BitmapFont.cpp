#pragma warning(disable : 4244)

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include "FontManager.h"
#include "BitmapFont.h"
#include <tier0/dbg.h>
#include <vgui/ISurface.h>
#include <tier0/mem.h>
#include <utlsymbol.h>
#include <utlbuffer.h>
#include "filesystem.h"
#include "plugins.h"

#include "tier0/memdbgon.h"

struct BitmapFontTable_t
{
	BitmapFontTable_t()
	{
		m_szName = UTL_INVAL_SYMBOL;
		m_pBitmapFont = NULL;
		m_pBitmapGlyphs = NULL;
		m_szTextureName[0] = 0;
	}

	CUtlSymbol m_szName;
	BitmapFont_t *m_pBitmapFont;
	BitmapGlyph_t *m_pBitmapGlyphs;
	char m_szTextureName[MAX_PATH];
};

static CUtlVector<BitmapFontTable_t> g_BitmapFontTable(1, 4);

CBitmapFont::CBitmapFont(void)
{
	m_scalex = 1.0f;
	m_scaley = 1.0f;

	m_bitmapFontHandle = g_BitmapFontTable.InvalidIndex();
}

CBitmapFont::~CBitmapFont(void)
{
}

bool CBitmapFont::Create(const char *pFontFilename, float scalex, float scaley, int flags)
{
	MEM_ALLOC_CREDIT();

	if (!pFontFilename || !pFontFilename[0])
		return false;

	char fontName[MAX_PATH];
	Q_FileBase(pFontFilename, fontName, MAX_PATH);
	Q_strlower(fontName);

	CUtlSymbol symbol = fontName;
	BitmapFontTable_t *pFontTable = NULL;

	for (int i = 0; i < g_BitmapFontTable.Count(); i++)
	{
		if (symbol == g_BitmapFontTable[i].m_szName)
		{
			m_bitmapFontHandle = i;
			pFontTable = &g_BitmapFontTable[m_bitmapFontHandle];
			break;
		}
	}

	if (!pFontTable)
	{
		FileHandle_t fh = g_pFileSystem->Open(pFontFilename, "rb", "GAME");

		if (fh == FILESYSTEM_INVALID_HANDLE)
		{
			return false;
		}

		size_t size = g_pFileSystem->Size(fh);
		BitmapFont_t *pBuf = (BitmapFont_t *)malloc(size);

		g_pFileSystem->Read(pBuf, size, fh);
		g_pFileSystem->Close(fh);

		if (pBuf->m_id != LittleLong(BITMAPFONT_ID) || pBuf->m_Version != LittleLong(BITMAPFONT_VERSION))
		{
			return false;
		}

		if (IsX360())
		{
			CByteswap swap;
			swap.ActivateByteSwapping(true);
			swap.SwapFieldsToTargetEndian((BitmapFont_t *)pBuf);
			swap.SwapFieldsToTargetEndian((BitmapGlyph_t *)((char *)pBuf + sizeof(BitmapFont_t)), ((BitmapFont_t *)pBuf)->m_NumGlyphs);
		}

		m_bitmapFontHandle = g_BitmapFontTable.AddToTail();

		pFontTable = &g_BitmapFontTable[m_bitmapFontHandle];
		pFontTable->m_szName = fontName;

		pFontTable->m_pBitmapFont = new BitmapFont_t;
		memcpy(pFontTable->m_pBitmapFont, pBuf, sizeof(BitmapFont_t));

		pFontTable->m_pBitmapGlyphs = new BitmapGlyph_t[pFontTable->m_pBitmapFont->m_NumGlyphs];
		memcpy(pFontTable->m_pBitmapGlyphs, (unsigned char *)pBuf + sizeof(BitmapFont_t), pFontTable->m_pBitmapFont->m_NumGlyphs * sizeof(BitmapGlyph_t));

		free(pBuf);

		Q_snprintf(pFontTable->m_szTextureName, MAX_PATH, "vgui/fonts/%s", fontName);
	}

	m_scalex = scalex;
	m_scaley = scaley;
	m_iFlags = vgui::ISurface::FONTFLAG_BITMAP;

	int bitmapFlags = pFontTable->m_pBitmapFont->m_Flags;

	if (bitmapFlags & BF_ANTIALIASED)
	{
		m_iFlags |= vgui::ISurface::FONTFLAG_ANTIALIAS;
	}

	if (bitmapFlags & BF_ITALIC)
	{
		m_iFlags |= vgui::ISurface::FONTFLAG_ITALIC;
	}

	if (bitmapFlags & BF_BLURRED)
	{
		m_iFlags |= vgui::ISurface::FONTFLAG_GAUSSIANBLUR;
		m_iBlur = 1;
	}

	if (bitmapFlags & BF_SCANLINES)
	{
		m_iScanLines = 1;
	}

	if (bitmapFlags & BF_OUTLINED)
	{
		m_iFlags |= vgui::ISurface::FONTFLAG_OUTLINE;
		m_iOutlineSize = 1;
	}

	if (bitmapFlags & BF_DROPSHADOW)
	{
		m_iFlags |= vgui::ISurface::FONTFLAG_DROPSHADOW;
		m_iDropShadowOffset = 1;
	}

	if (flags & vgui::ISurface::FONTFLAG_ADDITIVE)
	{
		m_bAdditive = true;
		m_iFlags |= vgui::ISurface::FONTFLAG_ADDITIVE;
	}

	m_iMaxCharWidth = (float)pFontTable->m_pBitmapFont->m_MaxCharWidth * m_scalex;
	m_iHeight = (float)pFontTable->m_pBitmapFont->m_MaxCharHeight * m_scaley;
	m_iAscent = (float)pFontTable->m_pBitmapFont->m_Ascent * m_scaley;
	m_szName = symbol;

	return true;
}

bool CBitmapFont::IsEqualTo(const char *windowsFontName, float scalex, float scaley, int flags)
{
	char fontname[MAX_PATH];
	Q_FileBase(windowsFontName, fontname, MAX_PATH);

	if (!Q_stricmp(fontname, m_szName.String()) && m_scalex == scalex && m_scaley == scaley)
	{
		int commonFlags = m_iFlags & flags;

		if (commonFlags & vgui::ISurface::FONTFLAG_ADDITIVE)
		{
			return true;
		}
	}

	return false;
}

void CBitmapFont::SetScale(float sx, float sy)
{
	m_scalex = sx;
	m_scaley = sy;
}

void CBitmapFont::GetCharABCWidths(int ch, int &a, int &b, int &c)
{
	Assert(IsValid() && ch >= 0 && ch <= 255);

	BitmapFontTable_t *pFont = &g_BitmapFontTable[m_bitmapFontHandle];

	ch = pFont->m_pBitmapFont->m_TranslateTable[ch];
	a = (float)pFont->m_pBitmapGlyphs[ch].a * m_scalex;
	b = (float)pFont->m_pBitmapGlyphs[ch].b * m_scalex;
	c = (float)pFont->m_pBitmapGlyphs[ch].c * m_scalex;
}

void CBitmapFont::GetCharCoords(int ch, float *left, float *top, float *right, float *bottom)
{
	Assert(IsValid() && ch >= 0 && ch <= 255);

	BitmapFontTable_t *pFont = &g_BitmapFontTable[m_bitmapFontHandle];

	ch = pFont->m_pBitmapFont->m_TranslateTable[ch];
	*left = (float)pFont->m_pBitmapGlyphs[ch].x / (float)pFont->m_pBitmapFont->m_PageWidth;
	*top = (float)pFont->m_pBitmapGlyphs[ch].y / (float)pFont->m_pBitmapFont->m_PageHeight;
	*right = (float)(pFont->m_pBitmapGlyphs[ch].x + pFont->m_pBitmapGlyphs[ch].w) /(float)pFont->m_pBitmapFont->m_PageWidth;
	*bottom = (float)(pFont->m_pBitmapGlyphs[ch].y + pFont->m_pBitmapGlyphs[ch].h) /(float)pFont->m_pBitmapFont->m_PageHeight;
}

const char *CBitmapFont::GetTextureName(void)
{
	Assert(IsValid());

	return g_BitmapFontTable[m_bitmapFontHandle].m_szTextureName;
}

BEGIN_BYTESWAP_DATADESC(BitmapGlyph_t)
	DEFINE_FIELD(x, FIELD_SHORT),
	DEFINE_FIELD(y, FIELD_SHORT),
	DEFINE_FIELD(w, FIELD_SHORT),
	DEFINE_FIELD(h, FIELD_SHORT),
	DEFINE_FIELD(a, FIELD_SHORT),
	DEFINE_FIELD(b, FIELD_SHORT),
	DEFINE_FIELD(c, FIELD_SHORT),
END_BYTESWAP_DATADESC()

BEGIN_BYTESWAP_DATADESC(BitmapFont_t)
	DEFINE_FIELD(m_id, FIELD_INTEGER),
	DEFINE_FIELD(m_Version, FIELD_INTEGER),
	DEFINE_FIELD(m_PageWidth, FIELD_SHORT),
	DEFINE_FIELD(m_PageHeight, FIELD_SHORT),
	DEFINE_FIELD(m_MaxCharWidth, FIELD_SHORT),
	DEFINE_FIELD(m_MaxCharHeight, FIELD_SHORT),
	DEFINE_FIELD(m_Flags, FIELD_SHORT),
	DEFINE_FIELD(m_Ascent, FIELD_SHORT),
	DEFINE_FIELD(m_NumGlyphs, FIELD_SHORT),
	DEFINE_ARRAY(m_TranslateTable, FIELD_CHARACTER, 256),
END_BYTESWAP_DATADESC()
