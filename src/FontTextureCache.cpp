#include <malloc.h>
#include "FontTextureCache.h"
#include <IEngineSurface.h>
#include "vgui_internal.h"
#include "BitmapFont.h"
#include "LoadTGA.h"

CFontTextureCache g_FontTextureCache;

#define TEXTURE_PAGE_WIDTH 256
#define TEXTURE_PAGE_HEIGHT 256

int CFontTextureCache::s_pFontPageSize[FONT_PAGE_SIZE_COUNT] =
{
	16,
	32,
	64,
	128,
	256
};

CFontTextureCache::CFontTextureCache(void): m_CharCache(0, 256, CacheEntryLessFunc)
{
	Clear();
}

CFontTextureCache::~CFontTextureCache(void)
{
}

void CFontTextureCache::Clear()
{
	m_CharCache.RemoveAll();
	m_PageList.RemoveAll();

	CacheEntry_t listHead = { 0, 0 };
	m_LRUListHeadIndex = m_CharCache.Insert(listHead);

	m_CharCache[m_LRUListHeadIndex].nextEntry = m_LRUListHeadIndex;
	m_CharCache[m_LRUListHeadIndex].prevEntry = m_LRUListHeadIndex;

	for (int i = 0; i < FONT_PAGE_SIZE_COUNT; ++i)
		m_pCurrPage[i] = -1;

	m_FontPages.SetLessFunc(DefLessFunc(vgui::HFont));
	m_FontPages.RemoveAll();
}

extern IEngineSurface *staticSurface;

bool CFontTextureCache::GetTextureForChar(HFont font, wchar_t wch, int *textureID, float **texCoords)
{
	if (FontManager().IsBitmapFont(font))
	{
		CBitmapFont *pWinFont = reinterpret_cast<CBitmapFont *>(FontManager().GetFontForChar(font, wch));

		if (!pWinFont)
			return false;

		float left, top, right, bottom;
		pWinFont->GetCharCoords(wch, &left, &top, &right, &bottom);

		static float sTexCoords[4];
		sTexCoords[0] = left;
		sTexCoords[1] = top;
		sTexCoords[2] = right;
		sTexCoords[3] = bottom;

		int index = m_FontPages.Find(font);
		Page_t *pPage;

		if (index == m_FontPages.InvalidIndex())
		{
			index = m_FontPages.Insert(font);

			pPage = &m_FontPages.Element(index);
			pPage->textureID = staticSurface->createNewTextureID();

			staticSurface->drawSetTexture(pPage->textureID);
			staticSurface->drawSetTextureFile(pPage->textureID, pWinFont->GetTextureName(), true, true);
		}
		else
		{
			pPage = &m_FontPages.Element(index);
		}

		*textureID = pPage->textureID;
		*texCoords = sTexCoords;
		return true;
	}
	else
	{
		static CacheEntry_t cacheitem;
		cacheitem.font = font;
		cacheitem.wch = wch;

		HCacheEntry cacheHandle = m_CharCache.Find(cacheitem);

		if (m_CharCache.IsValidIndex(cacheHandle))
		{
			int page = m_CharCache[cacheHandle].page;
			*textureID = m_PageList[page].textureID;
			*texCoords = m_CharCache[cacheHandle].texCoords;
			return true;
		}

		CWin32Font *winFont = FontManager().GetFontForChar(font, wch);

		if (!winFont)
			return false;

		int a, b, c;
		winFont->GetCharABCWidths(wch, a, b, c);

		int fontWide = max(b, 1);
		int fontTall = max(winFont->GetHeight(), 1);

		if (winFont->GetUnderlined())
		{
			fontWide += (a + c);
		}

		int page, drawX, drawY, twide, ttall;

		if (!AllocatePageForChar(fontWide, fontTall, page, drawX, drawY, twide, ttall))
			return false;

		int nByteCount = s_pFontPageSize[FONT_PAGE_SIZE_COUNT - 1] * s_pFontPageSize[FONT_PAGE_SIZE_COUNT - 1] * 4;
		unsigned char *rgba = (unsigned char *)_alloca(nByteCount);
		memset(rgba, 0, nByteCount);
		winFont->GetCharRGBA(wch, 0, 0, fontWide, fontTall, rgba);

		staticSurface->drawSetTexture(m_PageList[page].textureID);
		staticSurface->drawSetSubTextureRGBA(m_PageList[page].textureID, drawX, drawY, rgba, fontWide, fontTall);

		cacheitem.page = page;

		double adjust = 0.0f;
		cacheitem.texCoords[0] = (float)((double)drawX / ((double)twide + adjust));
		cacheitem.texCoords[1] = (float)((double)drawY / ((double)ttall + adjust));
		cacheitem.texCoords[2] = (float)((double)(drawX + fontWide) / (double)twide);
		cacheitem.texCoords[3] = (float)((double)(drawY + fontTall) / (double)ttall);
		m_CharCache.Insert(cacheitem);

		*textureID = m_PageList[page].textureID;
		*texCoords = cacheitem.texCoords;
		return true;
	}

	return false;
}

bool CFontTextureCache::AllocatePageForChar(int charWide, int charTall, int &pageIndex, int &drawX, int &drawY, int &twide, int &ttall)
{
	int nPageType = ComputePageType(charTall);
	pageIndex = m_pCurrPage[nPageType];

	int nNextX = 0;
	bool bNeedsNewPage = true;

	if (pageIndex > -1)
	{
		Page_t &page = m_PageList[pageIndex];
		nNextX = page.nextX + charWide;

		if (nNextX > page.wide)
		{
			page.nextX = 0;
			nNextX = charWide;
			page.nextY += page.fontHeight + 1;
		}

		bNeedsNewPage = ((page.nextY + page.fontHeight + 1) > page.tall);
	}

	if (bNeedsNewPage)
	{
		pageIndex = m_PageList.AddToTail();
		Page_t &newPage = m_PageList[pageIndex];
		m_pCurrPage[nPageType] = pageIndex;

		newPage.textureID = staticSurface->createNewTextureID();
		newPage.fontHeight = s_pFontPageSize[nPageType];
		newPage.wide = TEXTURE_PAGE_WIDTH;
		newPage.tall = TEXTURE_PAGE_HEIGHT;
		newPage.nextX = 0;
		newPage.nextY = 0;

		nNextX = charWide;

		unsigned char rgba[TEXTURE_PAGE_WIDTH * TEXTURE_PAGE_HEIGHT * 4];
		memset(rgba, 0, sizeof(rgba));
		staticSurface->drawSetTextureRGBA(newPage.textureID, rgba, newPage.wide, newPage.tall, false, true);
	}

	Page_t &page = m_PageList[pageIndex];
	drawX = page.nextX;
	drawY = page.nextY;
	twide = page.wide;
	ttall = page.tall;

	page.nextX = nNextX + 1;
	return true;
}

int CFontTextureCache::ComputePageType(int charTall) const
{
	for (int i = 0; i < FONT_PAGE_SIZE_COUNT; ++i)
	{
		if (charTall < s_pFontPageSize[i])
			return i;
	}

	return -1;
}

bool CFontTextureCache::CacheEntryLessFunc(CacheEntry_t const &lhs, CacheEntry_t const &rhs)
{
	if (lhs.font < rhs.font)
		return true;
	else if (lhs.font > rhs.font)
		return false;

	return (lhs.wch < rhs.wch);
}