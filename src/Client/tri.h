#ifndef TRI_H
#define TRI_H

struct FogParameters
{
	float color[3];
	float density;
	bool affectsSkyBox;
};

extern FogParameters g_FogParameters;

#endif