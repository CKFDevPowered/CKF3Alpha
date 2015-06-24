#include "hud.h"
#include "game_shared/voice_status.h"

CHud gHUD;

void HUD_VoiceStatus(int entindex, qboolean bTalking)
{
	if (entindex >= 0 && entindex < gEngfuncs.GetMaxClients())
	{
		if (bTalking)
		{
			g_PlayerExtraInfo[entindex].radarflashes = 99999;
			g_PlayerExtraInfo[entindex].radarflash = gHUD.m_flTime;
			g_PlayerExtraInfo[entindex].radarflashon = 1;
		}
		else
		{
			g_PlayerExtraInfo[entindex].radarflash = -1;
			g_PlayerExtraInfo[entindex].radarflashon = 0;
			g_PlayerExtraInfo[entindex].radarflashes = 0;
		}
	}

	GetClientVoice()->UpdateSpeakerStatus(entindex, bTalking);
}

void HUD_DirectorMessage(int iSize, void *pbuf)
{
	if (!gHUD.m_Spectator.DirectorMessage(iSize, pbuf))
		return;

	gExportfuncs.HUD_DirectorMessage(iSize, pbuf);
}