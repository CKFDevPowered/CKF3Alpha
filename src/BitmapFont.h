#ifndef _BITMAPFONT_H_
#define _BITMAPFONT_H_

#include "Win32Font.h"
#include "BitmapFontFile.h"

class ITexture;

class CBitmapFont : public CWin32Font
{
public:
	CBitmapFont(void);
	~CBitmapFont(void);

public:
	virtual bool Create(const char *windowsFontName, float scalex, float scaley, int flags);
	virtual bool IsEqualTo(const char *windowsFontName, float scalex, float scaley, int flags);
	virtual void GetCharABCWidths(int ch, int &a, int &b, int &c);

public:
	void GetCharCoords(int ch, float *left, float *top, float *right, float *bottom);
	void SetScale(float sx, float sy);
	const char *GetTextureName(void);

private:
	int m_bitmapFontHandle;
	float m_scalex;
	float m_scaley;
};

#endif