class CHintMessageQueue
{
public:
	void Reset(void);
	void Update(CBaseEntity *player);
	BOOL AddMessage(const char *message);
	BOOL IsEmpty(void) { return m_numMessages == 0; }

private:
	int m_numMessages;
	int m_sendMessages;
	int m_sizeMessages;
	float m_tmMessageEnd;
	const char *m_messages[8];
};