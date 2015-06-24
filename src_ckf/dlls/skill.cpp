#include "extdll.h"
#include "util.h"
#include "skill.h"

skilldata_t gSkillData;

float GetSkillCvar(char *pName)
{
	char szBuffer[64];
	int iCount = sprintf(szBuffer, "%s%d",pName, gSkillData.iSkillLevel);
	float flValue = CVAR_GET_FLOAT(szBuffer);

	if (flValue <= 0)
		ALERT(at_console, "\n\n** GetSkillCVar Got a zero for %s **\n\n", szBuffer);

	return flValue;
}