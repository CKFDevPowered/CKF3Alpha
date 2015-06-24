#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"

void CHintMessageQueue::Reset(void)
{
	m_numMessages = 0;
	m_sendMessages = 0;
	m_sizeMessages = 1;
	m_tmMessageEnd = 0;
	memset(m_messages, 0, sizeof(this->m_messages));
}

void CHintMessageQueue::Update(CBaseEntity *player)
{
	if (gpGlobals->time <= m_tmMessageEnd)
		return;

	if (IsEmpty())
		return;

	m_messages[m_sendMessages] = NULL;
	m_numMessages--;
	m_sendMessages = (m_sendMessages + 1) % 8;
	m_tmMessageEnd = gpGlobals->time + 6;

	if (m_messages[m_sendMessages])
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgHudText, NULL, player->pev);

		if (strlen(m_messages[m_sendMessages]) >= 500)
		{
			char tmp[500];
			strncpy(tmp, m_messages[m_sendMessages], 499);
			tmp[499] = '\0';
			WRITE_STRING(tmp);
		}
		else
			WRITE_STRING(m_messages[m_sendMessages]);

		WRITE_BYTE(1);
		MESSAGE_END();
	}
}

BOOL CHintMessageQueue::AddMessage(const char *message)
{
	if (m_numMessages >= 8)
		return FALSE;

	m_messages[m_sizeMessages] = message;
	m_sizeMessages = (m_sizeMessages + 1) % 8;
	m_numMessages++;
	return TRUE;
}