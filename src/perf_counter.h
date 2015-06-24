class CPerformanceCounter
{
public:
	void InitializePerformanceCounter(void);
	double GetCurTime(void);

public:
	int m_iLowShift;
	double m_flPerfCounterFreq;
	double m_flCurrentTime;
	double m_flLastCurrentTime;
};

extern CPerformanceCounter gPerformanceCounter;