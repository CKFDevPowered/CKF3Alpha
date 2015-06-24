#define VEHICLE_SPEED0_ACCELERATION 0.005
#define VEHICLE_SPEED1_ACCELERATION 0.002142857142857143
#define VEHICLE_SPEED2_ACCELERATION 0.003333333333333334
#define VEHICLE_SPEED3_ACCELERATION 0.004166666666666667
#define VEHICLE_SPEED4_ACCELERATION 0.004
#define VEHICLE_SPEED5_ACCELERATION 0.0038
#define VEHICLE_SPEED6_ACCELERATION 0.0045
#define VEHICLE_SPEED7_ACCELERATION 0.00425
#define VEHICLE_SPEED8_ACCELERATION 0.002666666666666667
#define VEHICLE_SPEED9_ACCELERATION 0.002285714285714286
#define VEHICLE_SPEED10_ACCELERATION 0.001875
#define VEHICLE_SPEED11_ACCELERATION 0.001444444444444444
#define VEHICLE_SPEED12_ACCELERATION 0.0012
#define VEHICLE_SPEED13_ACCELERATION 0.0009166666666666666
#define VEHICLE_SPEED14_ACCELERATION 0.001444444444444444

class CFuncVehicle : public CBaseEntity
{
public:
	void Spawn(void);
	void Restart(void);
	int Classify(void);
	void Precache(void);
	void Blocked(CBaseEntity *pOther);
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	void KeyValue(KeyValueData *pkvd);
	int ObjectCaps(void) { return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_DIRECTIONAL_USE; }
	void OverrideReset(void);
	void CheckTurning(void);
	void CollisionDetection(void);
	void TerrainFollowing(void);

public:
	void EXPORT Next(void);
	void EXPORT Find(void);
	void EXPORT NearestPath(void);
	void EXPORT DeadEnd(void);

public:
	void NextThink(float thinkTime, BOOL alwaysThink);
	void SetTrack(CPathTrack *track) { m_ppath = track->Nearest(pev->origin); }
	void SetControls(entvars_t *pevControls);
	BOOL OnControls(entvars_t *pev);
	void StopSound(void);
	void UpdateSound(void);

public:
	static CFuncVehicle *Instance(edict_t *pent);

public:
	CPathTrack *m_ppath;
	float m_length;
	float m_width;
	float m_height;
	float m_speed;
	float m_dir;
	float m_startSpeed;
	Vector m_controlMins;
	Vector m_controlMaxs;
	int m_soundPlaying;
	int m_sounds;
	int m_acceleration;
	float m_flVolume;
	float m_flBank;
	float m_oldSpeed;
	int m_iTurning;
	float m_flCheckTurningTime;
	float m_flCheckSpeedTime;
	float m_deceleration;
	float m_flStartGallopTime;
	float m_flNextTurningTime;
	float m_flNextUpdateSound;
	Vector m_vecRightBottom;
	Vector m_vecBottom;
	Vector m_vecLeftBottom;
	Vector m_vecRightTop;
	Vector m_vecTop;
	Vector m_vecLeftTop;
	Vector m_planeNormal;
	Vector m_direction;
	CBaseEntity *m_pDriver;

private:
	unsigned short m_usAdjustPitch;
};