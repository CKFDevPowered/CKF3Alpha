#ifndef DIRECTORCMD_H
#define DIRECTORCMD_H

#include "BitBuffer.h"

class DirectorCmd
{
public:
	DirectorCmd(void);
	~DirectorCmd(void);

public:
	float m_Time;
	int m_Type;
	int m_Size;
	BitBuffer m_Data;
	int m_Index;
};

#endif