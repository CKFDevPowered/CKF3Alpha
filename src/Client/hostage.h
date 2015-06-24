#define MAX_HOSTAGES 24

struct hostage_info_t
{
	bool dead;
	Vector origin;
	int health;
	float radarflash;
	int radarflashon;
	int radarflashes;
	char teamname[MAX_TEAM_NAME];
};

extern hostage_info_t *g_HostageInfo;