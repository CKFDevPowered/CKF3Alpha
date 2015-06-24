#include <locale.h>
#include "BitmapFont.h"
#include "FontManager.h"
#include <tier0/dbg.h>
#include <vgui/ISurface.h>
#include "vgui_internal.h"

#include <tier0/memdbgon.h>

static CFontManager s_FontManager;

CFontManager &FontManager(void)
{
	return s_FontManager;
}

CFontManager::CFontManager(void)
{
	m_FontAmalgams.EnsureCapacity(100);
	m_FontAmalgams.AddToTail();
	m_Win32Fonts.EnsureCapacity(100);

	setlocale(LC_CTYPE, "");
	setlocale(LC_TIME, "");
	setlocale(LC_COLLATE, "");
	setlocale(LC_MONETARY, "");
}

CFontManager::~CFontManager(void)
{
	ClearAllFonts();
}

void CFontManager::SetLanguage(const char *lang)
{
	strncpy(m_szLanguage, lang, sizeof(m_szLanguage) - 1);
	m_szLanguage[sizeof(m_szLanguage) - 1] = 0;
}

void CFontManager::ClearAllFonts(void)
{
	for (int i = 0; i < m_Win32Fonts.Count(); i++)
		delete m_Win32Fonts[i];

	m_Win32Fonts.RemoveAll();
}

vgui::HFont CFontManager::CreateFont(void)
{
	int i = m_FontAmalgams.AddToTail();
	return i;
}

bool CFontManager::AddGlyphSetToFont(HFont font, const char *windowsFontName, int tall, int weight, int blur, int scanlines, int flags, int lowRange, int highRange)
{
	if (m_FontAmalgams[font].GetCount() > 0)
		return false;

	CWin32Font *winFont = CreateOrFindWin32Font(windowsFontName, tall, weight, blur, scanlines, flags);

	do
	{
		if (IsFontForeignLanguageCapable(windowsFontName))
		{
			if (winFont)
			{
				m_FontAmalgams[font].AddFont(winFont, 0x0, 0xFFFF);
				return true;
			}
		}
		else
		{
			const char *localizedFontName = GetForeignFallbackFontName();

			if (winFont && !stricmp(localizedFontName, windowsFontName))
			{
				m_FontAmalgams[font].AddFont(winFont, 0x0, 0xFFFF);
				return true;
			}

			CWin32Font *asianFont = CreateOrFindWin32Font(localizedFontName, tall, weight, blur, scanlines, flags);

			if (winFont && asianFont)
			{
				m_FontAmalgams[font].AddFont(winFont, 0x0, 0xFF);
				m_FontAmalgams[font].AddFont(asianFont, 0x100, 0xFFFF);
				return true;
			}
			else if (asianFont)
			{
				m_FontAmalgams[font].AddFont(asianFont, 0x0, 0xFFFF);
				return true;
			}
		}
	}
	while (NULL != (windowsFontName = GetFallbackFontName(windowsFontName)));

	return false;
}

bool CFontManager::AddBitmapFontGlyphSet(HFont font, const char *windowsFontName, float scalex, float scaley, int flags)
{
	if (m_FontAmalgams[font].GetCount() > 0)
		m_FontAmalgams[font].RemoveAll();

	CBitmapFont *winFont = CreateOrFindBitmapFont(windowsFontName, scalex, scaley, flags);

	if (winFont)
	{
		m_FontAmalgams[font].AddFont(winFont, 0x0000, 0x00FF);
		return true;
	}

	return false;
}

void CFontManager::SetFontScale(vgui::HFont font, float sx, float sy)
{
	m_FontAmalgams[font].SetFontScale(sx, sy);
}

HFont CFontManager::GetFontByName(const char *name)
{
	for (int i = 1; i < m_FontAmalgams.Count(); i++)
	{
		if (!stricmp(name, m_FontAmalgams[i].Name()))
			return i;
	}

	return 0;
}

CWin32Font *CFontManager::GetFontForChar(HFont font, wchar_t wch)
{
	return m_FontAmalgams[font].GetFontForChar(wch);
}

void CFontManager::GetCharABCwide(HFont font, int ch, int &a, int &b, int &c)
{
	CWin32Font *winFont = m_FontAmalgams[font].GetFontForChar(ch);

	if (winFont)
	{
		winFont->GetCharABCWidths(ch, a, b, c);
	}
	else
	{
		a = c = 0;
		b = m_FontAmalgams[font].GetFontMaxWidth();
	}
}

int CFontManager::GetFontTall(HFont font)
{
	return m_FontAmalgams[font].GetFontHeight();
}

int CFontManager::GetFontAscent(HFont font, wchar_t wch)
{
	CWin32Font *winFont = m_FontAmalgams[font].GetFontForChar(wch);

	if (winFont)
		return winFont->GetAscent();
	else
		return 0;
}

bool CFontManager::IsFontAdditive(HFont font)
{
	return (m_FontAmalgams[font].GetFlags(0) & vgui::ISurface::FONTFLAG_ADDITIVE) ? true : false;
}

bool CFontManager::IsBitmapFont(HFont font)
{
	return (m_FontAmalgams[font].GetFlags(0) & vgui::ISurface::FONTFLAG_BITMAP) ? true : false;
}

int CFontManager::GetCharacterWidth(vgui::HFont font, int ch)
{
	if (iswprint(ch))
	{
		int a, b, c;
		GetCharABCwide(font, ch, a, b, c);
		return (a + b + c);
	}

	return 0;
}

void CFontManager::GetTextSize(vgui::HFont font, const wchar_t *text, int &wide, int &tall)
{
	wide = 0;
	tall = 0;

	if (!text)
		return;

	tall = GetFontTall(font);

	int xx = 0;

	for (int i = 0; ; i++)
	{
		wchar_t ch = text[i];

		if (ch == 0)
		{
			break;
		}
		else if (ch == '\n')
		{
			tall += GetFontTall(font);
			xx = 0;
		}
		else if (ch == '&')
		{

		}
		else
		{
			xx += GetCharacterWidth(font, ch);

			if (xx > wide)
				wide = xx;
		}
	}
}

struct FallbackFont_t
{
	const char *font;
	const char *fallbackFont;
};

const char *g_szValidAsianFonts[] = { "Marlett", NULL };

FallbackFont_t g_FallbackFonts[] =
{
	{ "Times New Roman", "Courier New" },
	{ "Courier New", "Courier" },
	{ "Verdana", "Arial" },
	{ "Trebuchet MS", "Arial" },
	{ "Tahoma", NULL },
	{ NULL, "Tahoma" },
};

bool CFontManager::IsFontForeignLanguageCapable(const char *windowsFontName)
{
	for (int i = 0; g_szValidAsianFonts[i] != NULL; i++)
	{
		if (!stricmp(g_szValidAsianFonts[i], windowsFontName))
			return true;
	}

	return false;
}

const char *CFontManager::GetFallbackFontName(const char *windowsFontName)
{
	int i;

	for (i = 0; g_FallbackFonts[i].font != NULL; i++)
	{
		if (!stricmp(g_FallbackFonts[i].font, windowsFontName))
			return g_FallbackFonts[i].fallbackFont;
	}

	return g_FallbackFonts[i].fallbackFont;
}

struct Win98ForeignFallbackFont_t
{
	const char *language;
	const char *fallbackFont;
};

Win98ForeignFallbackFont_t g_Win98ForeignFallbackFonts[] =
{
	{ "russian", "system" },
	{ "japanese", "win98japanese" },
	{ "thai", "system" },
	{ NULL, "Tahoma" },
};

extern bool s_bSupportsUnicode;

const char *CFontManager::GetForeignFallbackFontName(void)
{
	if (s_bSupportsUnicode)
		return "Tahoma";

	int i;

	for (i = 0; g_Win98ForeignFallbackFonts[i].language != NULL; i++)
	{
		if (!stricmp(g_Win98ForeignFallbackFonts[i].language, m_szLanguage))
			return g_Win98ForeignFallbackFonts[i].fallbackFont;
	}

	return g_Win98ForeignFallbackFonts[i].fallbackFont;
}

bool CFontManager::GetFontUnderlined(vgui::HFont font)
{
	return m_FontAmalgams[font].GetUnderlined();
}

CWin32Font *CFontManager::CreateOrFindWin32Font(const char *windowsFontName, int tall, int weight, int blur, int scanlines, int flags)
{
	CWin32Font *winFont = NULL;
	int i;

	for (i = 0; i < m_Win32Fonts.Count(); i++)
	{
		if (m_Win32Fonts[i]->IsEqualTo(windowsFontName, tall, weight, blur, scanlines, flags))
		{
			winFont = m_Win32Fonts[i];
			break;
		}
	}

	if (!winFont)
	{
		MEM_ALLOC_CREDIT();

		i = m_Win32Fonts.AddToTail();
		m_Win32Fonts[i] = new CWin32Font();

		if (m_Win32Fonts[i]->Create(windowsFontName, tall, weight, blur, scanlines, flags))
		{
			winFont = m_Win32Fonts[i];
		}
		else
		{
			delete m_Win32Fonts[i];
			m_Win32Fonts.Remove(i);
			return NULL;
		}
	}

	return winFont;
}

CBitmapFont *CFontManager::CreateOrFindBitmapFont(const char *windowsFontName, float scalex, float scaley, int flags)
{
	CBitmapFont *winFont = NULL;
	CBitmapFont *bitmapFont;
	int i;

	for (i = 0; i < m_Win32Fonts.Count(); i++)
	{
		CWin32Font *font = m_Win32Fonts[i];

		int testflags = font->GetFlags();

		if (~testflags & vgui::ISurface::FONTFLAG_BITMAP)
			continue;

		bitmapFont = reinterpret_cast<CBitmapFont *>(font);

		if (bitmapFont->IsEqualTo(windowsFontName, scalex, scaley, flags))
		{
			winFont = bitmapFont;
			break;
		}
	}

	if (!winFont)
	{
		MEM_ALLOC_CREDIT();

		i = m_Win32Fonts.AddToTail();
		bitmapFont = new CBitmapFont();

		if (bitmapFont->Create(windowsFontName, scalex, scaley, flags))
		{
			m_Win32Fonts[i] = bitmapFont;
			winFont = bitmapFont;
		}
		else
		{
			delete bitmapFont;
			m_Win32Fonts.Remove(i);
			return NULL;
		}
	}

	return winFont;
}

bool CFontManager::AddCustomFontFile(const char *fontFileName)
{
	return (::AddFontResource(fontFileName) > 0);
}