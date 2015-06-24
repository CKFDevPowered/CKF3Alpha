#include "hud.h"
#include "parsemsg.h"
#include "cl_util.h"
#include "ammohistory.h"

HistoryResource gHR;

#define AMMO_PICKUP_GAP (gHR.iHistoryGap + 5)
#define AMMO_PICKUP_PICK_HEIGHT (72 + (gHR.iHistoryGap * 2))
#define AMMO_PICKUP_HEIGHT_MAX (ScreenHeight - 100)

#define MAX_ITEM_NAME 32
int HISTORY_DRAW_TIME = 5;

struct ITEM_INFO
{
	char szName[MAX_ITEM_NAME];
	HSPRITE spr;
	wrect_t rect;
};

void HistoryResource::AddToHistory(int iType, int iId, int iCount)
{
	if (iType == HISTSLOT_AMMO && !iCount)
		return;

	if ((((AMMO_PICKUP_GAP * iCurrentHistorySlot) + AMMO_PICKUP_PICK_HEIGHT) > AMMO_PICKUP_HEIGHT_MAX) || (iCurrentHistorySlot >= MAX_HISTORY))
		iCurrentHistorySlot = 0;

	HIST_ITEM *freeslot = &rgAmmoHistory[iCurrentHistorySlot++];
	HISTORY_DRAW_TIME = CVAR_GET_FLOAT("hud_drawhistory_time");

	freeslot->type = iType;
	freeslot->iId = iId;
	freeslot->iCount = iCount;
	freeslot->DisplayTime = gHUD.m_flTime + HISTORY_DRAW_TIME;

	if (iType == HISTSLOT_AMMO)
	{
		HSPRITE *pSpr = gWR.GetAmmoPicFromWeapon(iId, freeslot->rc);
		freeslot->hSpr = pSpr ? *pSpr : NULL;
	}
	else if (iType == HISTSLOT_WEAP)
	{
		WEAPON *weap = gWR.GetWeapon(iId);

		if (weap)
		{
			freeslot->hSpr = weap->hInactive;
			freeslot->rc = weap->rcInactive;
		}
		else
			freeslot->hSpr = NULL;
	}
}

void HistoryResource::AddToHistory(int iType, const char *szName, int iCount)
{
	if (iType != HISTSLOT_ITEM)
		return;

	if ((((AMMO_PICKUP_GAP * iCurrentHistorySlot) + AMMO_PICKUP_PICK_HEIGHT) > AMMO_PICKUP_HEIGHT_MAX) || (iCurrentHistorySlot >= MAX_HISTORY))
		iCurrentHistorySlot = 0;

	HIST_ITEM *freeslot = &rgAmmoHistory[iCurrentHistorySlot++];

	int i = gHUD.GetSpriteIndex(szName);

	if (i == -1)
		return;

	freeslot->iId = i;
	freeslot->type = iType;
	freeslot->iCount = iCount;

	if (iType == HISTSLOT_AMMO)
	{
		HSPRITE *pSpr = gWR.GetAmmoPicFromWeapon(i, freeslot->rc);
		freeslot->hSpr = pSpr ? *pSpr : NULL;
	}
	else if (iType == HISTSLOT_WEAP)
	{
		WEAPON *weap = gWR.GetWeapon(i);

		if (weap)
		{
			freeslot->hSpr = weap->hInactive;
			freeslot->rc = weap->rcInactive;
		}
		else
			freeslot->hSpr = NULL;
	}

	HISTORY_DRAW_TIME = CVAR_GET_FLOAT("hud_drawhistory_time");
	freeslot->DisplayTime = gHUD.m_flTime + HISTORY_DRAW_TIME;
}

void HistoryResource::CheckClearHistory(void)
{
	for (int i = 0; i < MAX_HISTORY; i++)
	{
		if (rgAmmoHistory[i].type)
			return;
	}

	iCurrentHistorySlot = 0;
}

int HistoryResource::DrawAmmoHistory(float flTime)
{
	for (int i = 0; i < MAX_HISTORY; i++)
	{
		if (rgAmmoHistory[i].type)
		{
			rgAmmoHistory[i].DisplayTime = min(rgAmmoHistory[i].DisplayTime, gHUD.m_flTime + HISTORY_DRAW_TIME);

			if (rgAmmoHistory[i].DisplayTime <= flTime)
			{
				memset(&rgAmmoHistory[i], 0, sizeof(HIST_ITEM));
				CheckClearHistory();
			}
			else if (rgAmmoHistory[i].type == HISTSLOT_AMMO)
			{
				if (rgAmmoHistory[i].hSpr == NULL)
					return 1;

				int r, g, b;
				UnpackRGB(r, g, b, RGB_YELLOWISH);

				float scale = (rgAmmoHistory[i].DisplayTime - flTime) * 80;
				ScaleColors(r, g, b, min(scale, 255));

				int ypos = ScreenHeight - (AMMO_PICKUP_PICK_HEIGHT + (AMMO_PICKUP_GAP * i));
				int xpos = ScreenWidth - 24;

				SPR_Set(rgAmmoHistory[i].hSpr, r, g, b);
				SPR_DrawAdditive(0, xpos, ypos, &rgAmmoHistory[i].rc);

				gHUD.DrawHudNumberString(xpos - 10, ypos, xpos - 100, rgAmmoHistory[i].iCount, r, g, b);
			}
			else if (rgAmmoHistory[i].type == HISTSLOT_WEAP)
			{
				if (rgAmmoHistory[i].hSpr == NULL)
					return 1;

				WEAPON *weap = gWR.GetWeapon(rgAmmoHistory[i].iId);

				if (!weap)
					return 1;

				int r, g, b;
				UnpackRGB(r, g, b, RGB_YELLOWISH);

				if (!gWR.HasAmmo(weap))
					UnpackRGB(r, g, b, RGB_REDISH);

				float scale = (rgAmmoHistory[i].DisplayTime - flTime) * 80;
				ScaleColors(r, g, b, min(scale, 255));

				int ypos = ScreenHeight - (AMMO_PICKUP_PICK_HEIGHT + (AMMO_PICKUP_GAP * i));
				int xpos = ScreenWidth - (weap->rcInactive.right - weap->rcInactive.left);
				SPR_Set(rgAmmoHistory[i].hSpr, r, g, b);
				SPR_DrawAdditive(0, xpos, ypos, &rgAmmoHistory[i].rc);
			}
			else if (rgAmmoHistory[i].type == HISTSLOT_ITEM)
			{
				if (!rgAmmoHistory[i].iId)
					continue;

				wrect_t rect = gHUD.GetSpriteRect(rgAmmoHistory[i].iId);

				int r, g, b;
				UnpackRGB(r, g, b, RGB_YELLOWISH);

				float scale = (rgAmmoHistory[i].DisplayTime - flTime) * 80;
				ScaleColors(r, g, b, min(scale, 255));

				int ypos = ScreenHeight - (AMMO_PICKUP_PICK_HEIGHT + (AMMO_PICKUP_GAP * i));
				int xpos = ScreenWidth - (rect.right - rect.left) - 10;

				SPR_Set(gHUD.GetSprite(rgAmmoHistory[i].iId), r, g, b);
				SPR_DrawAdditive(0, xpos, ypos, &rect);
			}
		}
	}

	return 1;
}