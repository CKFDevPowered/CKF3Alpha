#ifndef VGUI_HUD_DEATHNOTICE_H
#define VGUI_HUD_DEATHNOTICE_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/controls.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/ProgressBar.h>
#include <VGUI/Vector.h>
#include "hud_element.h"

namespace vgui
{

class EditablePanel;
class ImagePanel;
class Label;

// Player entries in a death notice
struct DeathNoticePlayer
{
	DeathNoticePlayer()
	{
		szName[0] = 0;
		iTeam = TEAM_UNASSIGNED;
	}
	char		szName[MAX_PLAYER_NAME_LENGTH*2];	// big enough for player name and additional information
	int			iTeam;								// team #	
};

struct DeathNoticeIcon
{
	DeathNoticeIcon()
	{
		szName[0] = 0;
		iTextureID = 0;
		iWide = 0;
		iTall = 0;
		bColored = false;
	}
	char szName[32];
	int iTextureID;
	int iWide, iTall;
	bool bColored;
};

// Contents of each entry in our list of death notices
struct DeathNoticeItem 
{
	DeathNoticeItem() 
	{
		szIcon[0]=0;
		wzInfoText[0]=0;
		Icon = NULL;
		bSelfInflicted = false;
		flCreationTime = 0;
		bLocalPlayerInvolved = false;
		bCritKilled = false;
	}

	float GetExpiryTime();

	DeathNoticePlayer	Killer;
	DeathNoticePlayer   Victim;
	DeathNoticeIcon		*Icon;
	char		szIcon[32];		// name of icon to display
	wchar_t		wzInfoText[32];	// any additional text to display next to icon
	bool		bSelfInflicted;
	float		flCreationTime;
	bool		bLocalPlayerInvolved;
	bool		bCritKilled;
};

#define NUM_CORNER_COORD 10
#define NUM_BACKGROUND_COORD NUM_CORNER_COORD*4

class CTFHudDeathNotice : public EditablePanel, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CTFHudDeathNotice, EditablePanel );

public:
	CTFHudDeathNotice();

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

	virtual void Init(void);
	virtual void VidInit(void);
	virtual void Reset(void);
	virtual int FireMessage(const char *pszName, int iSize, void *pbuf);
	virtual bool ShouldDraw( void );

	virtual void OnThink(void);

	virtual void Paint();

	void RetireExpiredDeathNotices();
	int AddDeathNoticeItem();
	void DrawText( int x, int y, HFont hFont, Color clr, const wchar_t *szText );
	void GetBackgroundPolygonVerts( int x0, int y0, int x1, int y1, int iVerts, vgui::Vertex_t vert[] );
	void CalcRoundedCorners();
	void PlayRivalrySounds( int iKillerIndex, int iVictimIndex, int iType  );
	Color GetTeamColor( int iTeamNumber );
	void DrawUnicodeText( int x, int y, HFont hFont, Color clr, const wchar_t *szText );

	void LoadDeathIcon(const char *szName);
	DeathNoticeIcon *FindDeathIcon(const char *szName);

	int MsgFunc_DeathMsg(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_BuildDeath(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_ObjectMsg(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_Dominate(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_Revenge(const char *pszName, int iSize, void *pbuf);

	DECLARE_HUD_ELEMENT_SIMPLE();

private:
	CPanelAnimationVarAliasType( float, m_flLineHeight, "LineHeight", "16", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flLineSpacing, "LineSpacing", "4", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flCornerRadius, "CornerRadius", "3", "proportional_float" );
	CPanelAnimationVar( float, m_flMaxDeathNotices, "MaxDeathNotices", "4" );
	CPanelAnimationVar( bool, m_bRightJustify, "RightJustify", "1" );
	CPanelAnimationVar( vgui::HFont, m_hTextFont, "TextFont", "Default" );
	CPanelAnimationVar( Color, m_clrIcon, "IconColor", "HudWhite" );
	CPanelAnimationVar( Color, m_clrLocalPlayer, "LocalPlayerColor", "HUDBlack" );
	CPanelAnimationVar( Color, m_clrBaseBGColor, "BaseBackgroundColor", "46 43 42 220" );
	CPanelAnimationVar( Color, m_clrLocalBGColor, "LocalBackgroundColor", "245 229 196 200" );

	CPanelAnimationVar( Color, m_clrBlueText, "TeamBlue", "153 204 255 255" );
	CPanelAnimationVar( Color, m_clrRedText, "TeamRed", "255 64 64 255" );

	CUtlVector<DeathNoticeItem> m_DeathNotices;
	CUtlVector<DeathNoticeIcon> m_Icons;

	DeathNoticeIcon *m_CritIcon;

	Vector2D	m_CornerCoord[NUM_CORNER_COORD];
};
}

#endif