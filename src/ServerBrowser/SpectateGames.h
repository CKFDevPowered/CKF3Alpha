#ifndef SPECTATEGAMES_H
#define SPECTATEGAMES_H
#ifdef _WIN32
#pragma once
#endif

#include "InternetGames.h"

class CSpectateGames : public CInternetGames
{
public:
	CSpectateGames(vgui::Panel *parent);

protected:
	virtual void GetNewServerList(void);

private:
	typedef CInternetGames BaseClass;
};

#endif
