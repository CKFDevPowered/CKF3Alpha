#include <vgui/ISurface.h>

#include "memorybitmap.h"
#include "vgui_internal.h"

using namespace vgui;

MemoryBitmap::MemoryBitmap(unsigned char *texture, int wide, int tall)
{
	_texture = texture;
	_id = 0;
	_uploaded = false;
	_color = Color(255, 255, 255, 255);
	_pos[0] = _pos[1] = 0;
	_valid = true;
	_w = wide;
	_h = tall;

	ForceUpload(texture, wide, tall);
}

MemoryBitmap::~MemoryBitmap(void)
{
}

void MemoryBitmap::GetSize(int &wide, int &tall)
{
	wide = 0;
	tall = 0;

	if (!_valid)
		return;

	g_pSurface->DrawGetTextureSize(_id, wide, tall);
}

void MemoryBitmap::GetContentSize(int &wide, int &tall)
{
	GetSize(wide, tall);
}

void MemoryBitmap::SetSize(int x, int y)
{
}

void MemoryBitmap::SetPos(int x, int y)
{
	_pos[0] = x;
	_pos[1] = y;
}

void MemoryBitmap::SetColor(Color col)
{
	_color = col;
}

const char *MemoryBitmap::GetName(void)
{
	return "MemoryBitmap";
}

void MemoryBitmap::Paint(void)
{
	if (!_valid)
		return;

	if (!_id)
		_id = g_pSurface->CreateNewTextureID(true);

	if (!_uploaded)
		ForceUpload(_texture,_w,_h);

	g_pSurface->DrawSetTexture(_id);
	g_pSurface->DrawSetColor(_color[0], _color[1], _color[2], _color[3]);

	int wide, tall;
	GetSize(wide, tall);
	g_pSurface->DrawTexturedRect(_pos[0], _pos[1], _pos[0] + wide, _pos[1] + tall);
}

void MemoryBitmap::ForceUpload(unsigned char *texture, int wide, int tall)
{
	_texture = texture;
	_w = wide;
	_h = tall;

	if (!_valid)
		return;

	if (_w == 0 || _h == 0)
		return;

	if (!_id)
		_id = g_pSurface->CreateNewTextureID(true);

	g_pSurface->DrawSetTextureBGRA(_id, _texture, _w, _h, false, true);
	_uploaded = true;

	_valid = g_pSurface->IsTextureIDValid(_id);
}

HTexture MemoryBitmap::GetID(void)
{
	return _id;
}