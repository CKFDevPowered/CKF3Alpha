#include <vgui/VGUI.h>
#include "FontAmalgam.h"

#ifdef CreateFont
#undef CreateFont
#endif

using vgui::HFont;

class CWin32Font;

typedef struct
{
	char fontName[MAX_PATH];
	char fontFilePath[MAX_PATH];
}
fontCache_t;

class CWin32Font;
class CBitmapFont;

class CFontManager
{
public:
	CFontManager(void);
	~CFontManager(void);

public:
	void ClearAllFonts(void);
	HFont CreateFont(void);
	bool AddGlyphSetToFont(HFont font, const char *windowsFontName, int tall, int weight, int blur, int scanlines, int flags, int lowRange, int highRange);
	bool AddBitmapFontGlyphSet(HFont font, const char *windowsFontName, float scalex, float scaley, int flags);
	void SetFontScale(HFont font, float sx, float sy);
	HFont GetFontByName(const char *name);
	void GetCharABCwide(HFont font, int ch, int &a, int &b, int &c);
	int GetFontTall(HFont font);
	int GetFontAscent(HFont font, wchar_t wch);
	int GetCharacterWidth(HFont font, int ch);
	void GetTextSize(HFont font, const wchar_t *text, int &wide, int &tall);
	CWin32Font *GetFontForChar(HFont, wchar_t wch);
	bool IsFontAdditive(HFont font);
	bool IsBitmapFont(HFont font);
	void SetLanguage(const char *lang);
	bool IsFontForeignLanguageCapable(const char *windowsFontName);
	const char *GetFallbackFontName(const char *windowsFontName);
	const char *GetForeignFallbackFontName(void);
	bool GetFontUnderlined(HFont font);
	CWin32Font *CreateOrFindWin32Font(const char *windowsFontName, int tall, int weight, int blur, int scanlines, int flags);
	CBitmapFont *CreateOrFindBitmapFont(const char *windowsFontName, float scalex, float scaley, int flags);
	bool AddCustomFontFile(const char *fontFileName);

private:
	CUtlVector<CFontAmalgam> m_FontAmalgams;
	CUtlVector<CWin32Font *> m_Win32Fonts;
	char m_szLanguage[64];
};

extern CFontManager &FontManager(void);