#include <vgui/ISurface.h>
#include "bitmap.h"
#include "vgui_internal.h"
#include "FileSystem.h"
#include "UtlBuffer.h"
#include <tier0/dbg.h>

#include "tier0/memdbgon.h"

using namespace vgui;

Bitmap::Bitmap(const char *filename, bool hardwareFiltered)
{
	_filtered = hardwareFiltered;

	int size = strlen(filename) + 1;
	_filename = (char *)malloc(size);
	Assert(_filename);

	Q_snprintf(_filename, size, "%s", filename);

	_bProcedural = false;

	if (Q_stristr(filename, ".pic"))
		_bProcedural = true;

	_id = 0;
	_uploaded = false;
	_color = Color(255, 255, 255, 255);
	_pos[0] = _pos[1] = 0;
	_valid = true;
	_wide = 0;
	_tall = 0;

	ForceUpload();
}

Bitmap::~Bitmap(void)
{
	if (_filename)
		free(_filename);
}

void Bitmap::GetSize(int &wide, int &tall)
{
	wide = 0;
	tall = 0;

	if (!_valid)
		return;

	if (0 == _wide && 0 == _tall)
		g_pSurface->DrawGetTextureSize(_id, _wide, _tall);

	wide = _wide;
	tall = _tall;
}

void Bitmap::GetContentSize(int &wide, int &tall)
{
	GetSize(wide, tall);
}

void Bitmap::SetSize(int x, int y)
{
	_wide = x;
	_tall = y;
}

void Bitmap::SetPos(int x, int y)
{
	_pos[0] = x;
	_pos[1] = y;
}

void Bitmap::SetColor(Color col)
{
	_color = col;
}

const char *Bitmap::GetName(void)
{
	return _filename;
}

void Bitmap::Paint(void)
{
	if (!_valid)
		return;

	if (!_id)
		_id = g_pSurface->CreateNewTextureID();

	if (!_uploaded)
		ForceUpload();

	g_pSurface->DrawSetColor(_color[0], _color[1], _color[2], _color[3]);
	g_pSurface->DrawSetTexture(_id);

	if (_wide == 0)
		GetSize(_wide, _tall);

	g_pSurface->DrawTexturedRect(_pos[0], _pos[1], _pos[0] + _wide, _pos[1] + _tall);
}

void Bitmap::ForceUpload(void)
{
	if (!_valid || _uploaded)
		return;

	if (!_id)
		_id = g_pSurface->CreateNewTextureID(_bProcedural);

	if (!_bProcedural)
		g_pSurface->DrawSetTextureFile(_id, _filename, _filtered, false);

	_uploaded = true;
	_valid = g_pSurface->IsTextureIDValid(_id);
}

HTexture Bitmap::GetID(void)
{
	return _id;
}