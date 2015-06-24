#ifndef NODES_H
#define NODES_H

#define bits_NODE_GROUP_REALM 1

class CLink
{
public:
	entvars_t *m_pLinkEnt;
};

class CGraph
{
public:
	BOOL m_fGraphPresent;
	BOOL m_fGraphPointersSet;
	int m_cLinks;
	CLink *m_pLinkPool;

public:
	void InitGraph(void);
	int AllocNodes(void);
	int CheckNODFile(char *szMapName);
	int FLoadGraph(char *szMapName);
	int FSetGraphPointers(void);
	void ShowNodeConnections(int iNode);
	int FindNearestNode(const Vector &vecOrigin, CBaseEntity *pEntity);
	int FindNearestNode(const Vector &vecOrigin, int afNodeTypes);
};

extern CGraph WorldGraph;
#endif