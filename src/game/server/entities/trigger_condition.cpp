/***
 *
 *	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
 *
 *	This product contains software technology licensed from Id
 *	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
 *	All Rights Reserved.
 *
 *   Use, distribution, and modification of this source code and/or resulting
 *   object code is restricted to non-commercial enhancements to products from
 *   Valve LLC.  All other use, distribution, or modification is prohibited
 *   without written permission from Valve LLC.
 *
 ****/

#include "trigger_condition.h"

BEGIN_DATAMAP( CTriggerCondition )
	DEFINE_FIELD( m_iCheckType, FIELD_INTEGER ),
	DEFINE_FIELD( m_iszSourceKey, FIELD_STRING ),
	DEFINE_FIELD( m_iszValueName, FIELD_STRING ),
	DEFINE_FIELD( m_iszSourceName, FIELD_STRING ),
	DEFINE_FIELD( m_iszCheckValue, FIELD_STRING ),
	DEFINE_FIELD( m_fCheckInterval, FIELD_FLOAT ),
	DEFINE_FIELD( m_iCheckBehaviour, FIELD_INTEGER ),
END_DATAMAP();

LINK_ENTITY_TO_CLASS( trigger_condition, CTriggerCondition )

void CTriggerCondition :: Spawn()
{
    pev->solid = SOLID_NOT;
}

void CTriggerCondition :: Use( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value )
{
}

bool CTriggerCondition :: KeyValue( KeyValueData* pkvd )
{
    if( FStrEq( pkvd->szKeyName, "m_iCheckBehaviour" ) )
    {
        m_iCheckBehaviour = atoi( pkvd->szValue );
    }
    else if( FStrEq( pkvd->szKeyName, "m_iCheckType" ) )
    {
        m_iCheckType = atoi( pkvd->szValue );
    }
    else if( FStrEq( pkvd->szKeyName, "m_fCheckInterval" ) )
    {
        m_fCheckInterval = atof( pkvd->szValue );
    }
    else if( FStrEq( pkvd->szKeyName, "m_iszSourceKey" ) )
    {
        m_iszSourceKey = ALLOC_STRING( pkvd->szValue );
    }
    else if( FStrEq( pkvd->szKeyName, "m_iszValueName" ) )
    {
        m_iszValueName = ALLOC_STRING( pkvd->szValue );
    }
    else if( FStrEq( pkvd->szKeyName, "m_iszSourceName" ) )
    {
        m_iszSourceName = ALLOC_STRING( pkvd->szValue );
    }
    else if( FStrEq( pkvd->szKeyName, "m_iszCheckValue" ) )
    {
        m_iszCheckValue = ALLOC_STRING( pkvd->szValue );
    }
    else
    {
        return CPointEntity::KeyValue(pkvd);
    }
    return true;
}
