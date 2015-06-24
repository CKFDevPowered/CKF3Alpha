#include "hud.h"
#include "cdll_dll.h"
#include "perf_counter.h"

#include "buy_preset_debug.h"
#include "buy_presets.h"
#include "weapon_csbase.h"
#include "filesystem.h"
#include <vgui/ILocalize.h>
#include <vgui_controls/Controls.h>
#include "plugins.h"

BuyPresetManager *TheBuyPresets = NULL;

static void PrintBuyPresetUsage(void)
{
	if (TheBuyPresets->GetNumPresets())
	{
		gEngfuncs.Con_Printf("usage:  cl_buy_favorite <1...%d>\n", TheBuyPresets->GetNumPresets());
	}
	else
		gEngfuncs.Con_Printf("cl_buy_favorite: no favorites are defined\n");
}

void cl_buy_favorite_f(void)
{
#if USE_BUY_PRESETS
	if (gEngfuncs.Cmd_Argc() != 2)
	{
		PRESET_DEBUG("cl_buy_favorite: no favorite specified\n");
		PrintBuyPresetUsage();
		return;
	}

	int presetIndex = atoi(gEngfuncs.Cmd_Argv(1)) - 1;

	if (presetIndex < 0 || presetIndex >= TheBuyPresets->GetNumPresets())
	{
		PRESET_DEBUG("cl_buy_favorite: favorite %d doesn't exist\n", presetIndex);
		PrintBuyPresetUsage();
		return;
	}

	TheBuyPresets->PurchasePreset(presetIndex);
#endif
}

void cl_buy_favorite_set_f(void)
{
#if USE_BUY_PRESETS
	if (gEngfuncs.Cmd_Argc() != 2)
	{
		PRESET_DEBUG("cl_buy_favorite_set: no favorite specified\n");
		PrintBuyPresetUsage();
		return;
	}

	int presetIndex = atoi(gEngfuncs.Cmd_Argv(1)) - 1;

	if (presetIndex < 0 || presetIndex >= TheBuyPresets->GetNumPresets())
	{
		PRESET_DEBUG("cl_buy_favorite_set: favorite %d doesn't exist\n", presetIndex);
		PrintBuyPresetUsage();
		return;
	}

	TheBuyPresets->SetPurchasePreset(presetIndex);
#endif
}

void cl_buy_favorite_reset_f(void)
{
#if USE_BUY_PRESETS
	if ((g_iTeamNumber != TEAM_CT && g_iTeamNumber != TEAM_TERRORIST))
		return;

	TheBuyPresets->ResetEditToDefaults();
	TheBuyPresets->SetPresets(TheBuyPresets->GetEditPresets());
	TheBuyPresets->Save();
#endif
}

BuyPresetManager::BuyPresetManager(void)
{
	m_loadedTeam = TEAM_UNASSIGNED;

	gEngfuncs.pfnAddCommand("cl_buy_favorite", cl_buy_favorite_f);
	gEngfuncs.pfnAddCommand("cl_buy_favorite_set", cl_buy_favorite_set_f);
	gEngfuncs.pfnAddCommand("cl_buy_favorite_reset", cl_buy_favorite_reset_f);
}

void BuyPresetManager::VerifyLoadedTeam(void)
{
#if USE_BUY_PRESETS
	int playerTeam = g_iTeamNumber;

	if (playerTeam == m_loadedTeam)
		return;

	if (playerTeam != TEAM_CT && playerTeam != TEAM_TERRORIST)
		return;

	m_presets.RemoveAll();

	const char *filename = "cfg/BuyPresets_TER.vdf";

	if (playerTeam == TEAM_CT)
		filename = "cfg/BuyPresets_CT.vdf";

	KeyValues *data = new KeyValues("Presets");
	bool fileExists = data->LoadFromFile(g_pFileSystem, filename, NULL);
	KeyValues *presetKey = data->GetFirstSubKey();

	while (presetKey)
	{
		BuyPreset preset;
		preset.Parse(presetKey);
		m_presets.AddToTail(preset);
		presetKey = presetKey->GetNextKey();
	}

	if (!m_presets.Count())
	{
		const char *filename = "cfg/BuyPresetsDefault_TER.vdf";

		if (playerTeam == TEAM_CT)
			filename = "cfg/BuyPresetsDefault_CT.vdf";

		KeyValues *data, *presetKey;

		data = new KeyValues("Presets");
		data->LoadFromFile(g_pFileSystem, filename, NULL);
		presetKey = data->GetFirstSubKey();

		while (presetKey)
		{
			BuyPreset preset;
			preset.Parse(presetKey);
			m_presets.AddToTail(preset);

			presetKey = presetKey->GetNextKey();
		}

		data->deleteThis();
	}

	while (m_presets.Count() < NUM_PRESETS)
	{
		BuyPreset preset;
		m_presets.AddToTail(preset);
	}

	data->deleteThis();
	m_editPresets = m_presets;

	if (!fileExists)
		Save();
#endif
}

void BuyPresetManager::ResetEditToDefaults(void)
{
#if USE_BUY_PRESETS
	VerifyLoadedTeam();

	int playerTeam = g_iTeamNumber;

	if (playerTeam != TEAM_CT && playerTeam != TEAM_TERRORIST)
		return;

	m_editPresets.RemoveAll();

	const char *filename = "cfg/BuyPresetsDefault_TER.vdf";

	if (playerTeam == TEAM_CT)
		filename = "cfg/BuyPresetsDefault_CT.vdf";

	KeyValues *data;
	KeyValues *presetKey;
	data = new KeyValues("Presets");
	data->LoadFromFile(g_pFileSystem, filename, NULL);

	presetKey = data->GetFirstSubKey();

	while (presetKey)
	{
		BuyPreset preset;
		preset.Parse(presetKey);
		m_editPresets.AddToTail(preset);

		presetKey = presetKey->GetNextKey();
	}

	data->deleteThis();
#endif
}

void BuyPresetManager::Save(void)
{
#if USE_BUY_PRESETS
	const char *filename = "cfg/BuyPresets_TER.vdf";

	switch (g_iTeamNumber)
	{
		case TEAM_CT:
		{
			filename = "cfg/BuyPresets_CT.vdf";
			break;
		}

		case TEAM_TERRORIST:
		{
			filename = "cfg/BuyPresets_TER.vdf";
			break;
		}

		default: return;
	}

	KeyValues *data = new KeyValues("Presets");

	for (int i = 0; i < m_presets.Count(); ++i)
		m_presets[i].Save(data);

	data->SaveToFile(g_pFileSystem, filename, NULL);
	data->deleteThis();
#endif
}

const BuyPreset *BuyPresetManager::GetPreset(int index) const
{
	if (index < 0 || index >= m_presets.Count())
		return NULL;

	return &m_presets[index];
}

void BuyPresetManager::SetPreset(int index, const BuyPreset *preset)
{
	if (index < 0 || index >= m_presets.Count() || !preset)
		return;

	m_presets[index] = *preset;
}

BuyPreset *BuyPresetManager::GetEditPreset(int index)
{
	if (index < 0 || index >= m_editPresets.Count())
		return NULL;

	return &m_editPresets[index];
}

void BuyPresetManager::PurchasePreset(int presetIndex)
{
	if (presetIndex >= 0 && presetIndex < m_presets.Count())
	{
		const BuyPreset *preset = &(m_presets[presetIndex]);
		int setIndex;

		for (setIndex = 0; setIndex < preset->GetNumSets(); ++setIndex)
		{
			const WeaponSet *itemSet = preset->GetSet(setIndex);

			if (itemSet)
			{
				int currentCost = 1;

				if (currentCost > 0)
				{
					char buf[BUY_PRESET_COMMAND_LEN];
					itemSet->GenerateBuyCommands(buf);

					PRESET_DEBUG("%s\n", buf);
					engine->pfnClientCmd(buf);
					return;
				}
				else if (currentCost == 0)
				{
					return;
				}
			}
		}

		return;
	}

	PRESET_DEBUG("cl_buy_favorite: preset %d doesn't exist.\n", presetIndex);
}

void BuyPresetManager::SetPurchasePreset(int presetIndex)
{
	if (presetIndex >= 0 && presetIndex < m_presets.Count())
	{
		const BuyPreset *preset = TheBuyPresets->GetPreset(presetIndex);

		if (!preset)
			return;

		WeaponSet ws;
		TheBuyPresets->GetCurrentLoadout(&ws);
		BuyPreset newPreset(*preset);
		newPreset.ReplaceSet(0, ws);

		TheBuyPresets->SetPreset(presetIndex, &newPreset);
		TheBuyPresets->Save();
		return;
	}

	PRESET_DEBUG("cl_buy_favorite_set: favorite %d doesn't exist\n", presetIndex);
}