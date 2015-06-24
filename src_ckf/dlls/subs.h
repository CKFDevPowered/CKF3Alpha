#ifndef SUBS_H
#define SUBS_H

void CPointEntity :: Spawn( void )
{
	pev->solid = SOLID_NOT;
}

void CNullEntity :: Spawn( void )
{
	REMOVE_ENTITY(ENT(pev));
}

LINK_ENTITY_TO_CLASS(info_landmark,CPointEntity);
LINK_ENTITY_TO_CLASS(info_null,CNullEntity);
LINK_ENTITY_TO_CLASS(info_texlights,CNullEntity); // don't complain about Merl's new info entities
LINK_ENTITY_TO_CLASS(info_compile_parameters,CNullEntity);

LINK_ENTITY_TO_CLASS(info_player_deathmatch,CBaseDMStart);
LINK_ENTITY_TO_CLASS(info_player_start,CBaseDMStart);

void CBaseDMStart::Spawn(void)
{
	pev->solid = SOLID_NOT;
	m_iDisabled = 0;
}

void CBaseDMStart::Restart(void)
{
	m_iDisabled = 0;
}

void CBaseDMStart::KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "master"))
	{
		pev->netname = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CPointEntity::KeyValue( pkvd );
}

#endif