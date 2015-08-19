#ifdef CreateDialog
#undef CreateDialog
#endif

class IServerBrowser
{
public:
	virtual bool Active(void) = 0;
	virtual bool JoinGame(int serverIP, int serverPort) = 0;
	virtual bool JoinGame(void *gameList, unsigned short serverIndex) = 0;
	virtual bool OpenGameInfoDialog(void *gameList, unsigned int serverIndex) = 0;
	virtual void CloseGameInfoDialog(void *gameList, unsigned int serverIndex) = 0;
	virtual void CloseAllGameInfoDialogs(void) = 0;
	virtual void CreateDialog(void) = 0;
	virtual void Open(void) = 0;
};