#include <windows.h>
#include <stdio.h>

char *pdate = __DATE__;
char *ptime = __TIME__;

char *mon[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
char mond[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

char *build_number(void)
{
	int m = 0;
	int d = 0;
	int y = 0;
	static char result[32];
	static int b = 0;

	if (b)
		return result;

	for (m = 0; m < 11; m++)
	{
		if (!strnicmp(pdate, mon[m], 3))
			break;

		d += mond[m];
	}

	d += atoi(&pdate[4]) - 4;
	y = atoi(&pdate[7]) - 1900;
	b = d + (int)((y - 1) * 365.25);

	if (!(y % 4) && m > 1)
		b++;

	b -= 40966;
	sprintf(result, "%d", b);
	return result;
}