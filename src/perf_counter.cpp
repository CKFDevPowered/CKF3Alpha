#include <windows.h>
#include "perf_counter.h"
#include "tier0\fasttimer.h"

CPerformanceCounter gPerformanceCounter;

void CPerformanceCounter::InitializePerformanceCounter(void)
{
}

double CPerformanceCounter::GetCurTime(void)
{
	CCycleCount cnt;
	cnt.Sample();
	return (double)cnt.GetSeconds();
}